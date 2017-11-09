//
// Created by victor on 23/10/17.
//

#include <stdexcept>
#include "DbManager.h"
#include "RestDbConfiguration.h"
#include "EndpointDescriptor.h"
#include "Validations.h"
#include <folly/Singleton.h>

namespace restdbxx {
namespace {
struct DbManagerSingletonTag {};
folly::Singleton<DbManager, DbManagerSingletonTag> the_instance;

const std::string ENDPOINTS_KEY = "/__endpoints";
const std::string ENDPOINTS_COUNT_KEY = "/__endpoints/count";
const std::string USERS_KEY = "/__users";
const std::string TEST_ENDPOINT_KEY = "/test_endpoint";
}

std::shared_ptr<DbManager> DbManager::get_instance() {
  return the_instance.try_get();
}
DbManager::DbManager() {

  //_root = folly::dynamic::object();
  using google::GLOG_INFO;
  using google::GLOG_ERROR;
  //_root.
  VLOG(GLOG_INFO) << "Initializing DbManager";
  if (!is_initialized()) {
    perform_database_init_tasks();
  }
  const std::string& db_path = RestDbConfiguration::get_instance()->getDb_path();
  auto opts = rocksdb::Options();
  //auto dbOpts = rocksdb::DBOptions();
  opts.create_if_missing = true;
  opts.create_missing_column_families=true;
  opts.OptimizeForSmallDb();
  //dbOpts.
  std::vector<std::string> cfs;

  auto status = rocksdb::DB::ListColumnFamilies(opts, db_path, &cfs);
  if (!status.ok()) {
    VLOG(GLOG_ERROR) << status.ToString();
  }

  std::vector<rocksdb::ColumnFamilyDescriptor> descrs;
  const auto cfOpts = rocksdb::ColumnFamilyOptions(opts);
  for (const std::string& n: cfs) {
    descrs.emplace_back(n, cfOpts);
  }
  rocksdb::TransactionDB *db;
  status = rocksdb::TransactionDB::Open(opts, rocksdb::TransactionDBOptions(), db_path, const_cast<const std::vector<rocksdb::ColumnFamilyDescriptor>&>(descrs), &_handles, &db);
  if (!status.ok()) {
    VLOG(google::GLOG_ERROR) << status.ToString();
  }
  _db.reset(db);
  VLOG(GLOG_INFO) << "Listing column families";
  for (auto handle: _handles) {
    VLOG(GLOG_INFO) << handle->GetName();
    _cfh_map.emplace(handle->GetName(), handle);
  }
  VLOG(GLOG_INFO) << "End of column families";

  if (!is_endpoint(TEST_ENDPOINT_KEY)) {
    _db->Put(rocksdb::WriteOptions(), ENDPOINTS_COUNT_KEY, "0");
    VLOG(GLOG_INFO) << "Test endpoint inexistent, creating.";
    add_endpoint(TEST_ENDPOINT_KEY);
    add_endpoint(USERS_KEY);
  }

}
bool DbManager::path_exists(const std::string path) {

  std::string val;
  auto readOpts = rocksdb::ReadOptions();
  //bool may_exist = _db->KeyMayExist(readOpts, path, &val);
  //if (!may_exist) return false;
  //else {
  auto s = _db->Get(readOpts, path, &val);
  return !s.IsNotFound();
  //}
}

std::vector<std::string> DbManager::get_path_parts(const std::string path) const {
  std::vector<std::__cxx11::string> tokens;
  std::string path_copy = path;
  boost::erase_first(path_copy, "/");
  split(tokens, path_copy, boost::is_any_of("/"));
  return tokens;
}

void DbManager::post(const std::string path, folly::dynamic &data) {
  using google::GLOG_INFO;
  using google::GLOG_ERROR;

  VLOG(GLOG_INFO) << "Request to post to " << path;
  if (can_post(path)) {
    using rocksdb::DB;
    VLOG(GLOG_INFO) << "Can post to path: CONFIRMED";
    auto writeOpts = rocksdb::WriteOptions();
    auto readOpts = rocksdb::ReadOptions();
    //auto txn = _db->BeginTransaction(writeOpts);
    std::string next_id;
    VLOG(GLOG_INFO) << "Getting " << path;
    auto s = _db->Get(readOpts, _db->DefaultColumnFamily(), path, &next_id);
    if (!s.ok()) {
      VLOG(GLOG_ERROR) << s.ToString();
    }
    data.insert("ID", std::string(next_id));
    std::string new_path = path + "/" + next_id;
    std::string new_val = folly::toPrettyJson(data);

    rocksdb::WriteBatch batch;
    batch.Put(_cfh_map.at(path), new_path, new_val);
    int id = std::stoi(next_id);
    id++;
    batch.Put(path, std::to_string(id));
    s = _db->Write(rocksdb::WriteOptions(), &batch);
//auto s = txn->Commit();
    if (!s.ok()) {
      VLOG(GLOG_ERROR) << s.ToString();
    }
    //delete txn;

    /*std::vector<std::string> tokens;

    boost::split(tokens, path, boost::is_any_of("/"));
    LOG(INFO) << "Tokens: " << tokens.size() << " - " <<  tokens;
    folly::dynamic deep_obj = to_deep_object(tokens, data);
    LOG(INFO) << "Deep obj: " << folly::toPrettyJson(deep_obj);
    deep_merge(_root, deep_obj);*/
  } else
    VLOG(GLOG_INFO) << "Can post to path: DENIED";
}

void DbManager::put(const std::string path, const folly::dynamic &data) {
  LOG(INFO) << "Request to put to: " << path;
  auto pathClone = path;
  Validations::sanitize_path(pathClone);
  if (path_exists(pathClone)) {
    if (is_endpoint(path)) {
      rocksdb::Transaction *trx = _db->BeginTransaction(rocksdb::WriteOptions());

      auto new_id = get_endpoint_count_and_increment(path, trx);
      auto new_key = path + "/" + std::to_string(new_id);
      auto status = trx->Put(_cfh_map[path], new_key, folly::toPrettyJson(data));
      if (!status.ok()) {
        trx->Rollback();
        VLOG(google::GLOG_WARNING) << "no pude commitear la transaccion: " << status.ToString();
        return;
      }
    } else {
      _db->Put(rocksdb::WriteOptions(), path, folly::toJson(data));
    }
  }
}
void DbManager::remove(const std::string path) {
  //folly::make_exception_wrapper()
  if (path_exists(path)) {
    auto parts = get_path_parts(path);
  //  auto aux = &_root;

    for (auto part = parts.begin(); part != parts.end(); ++part) {
      if (std::distance(part, parts.end()) == 1) {
        //last part
        //aux->erase(*part);
        return;
      }
      /*if (aux->isObject()) {
        //aux = &aux->at(*part);
      } else {
        LOG(ERROR) << "uh oh";
      }*/
    }
  }
}
bool DbManager::can_post(const std::string path) {
  /*if (!path_exists(path)) return true;

  auto parts = get_path_parts(path);
  auto aux = &_root;
  for (auto p = parts.begin(); p != parts.end(); ++p) {
    if (std::distance(p, parts.end()) == 1) {
      return aux->isArray();
    }
    aux = &aux->at(*p);

  }
  return false;*/
  return is_endpoint(path);
}
DbManager::~DbManager() {
  for (auto h: _handles) {
    auto s = _db->DestroyColumnFamilyHandle(h);
    if (!s.ok()) LOG(ERROR) << s.ToString();
  }
  _db.reset();
}
void DbManager::add_endpoint(const std::string path) {
  using rocksdb::DB;
  using google::WARNING;
  std::string count_str;
  rocksdb::ColumnFamilyHandle *handle;

  auto cfOpts = rocksdb::ColumnFamilyOptions();
  auto status = _db->CreateColumnFamily(cfOpts, path, &handle);
  auto txn = _db->BeginTransaction(rocksdb::WriteOptions());
  txn->Get(rocksdb::ReadOptions(), ENDPOINTS_COUNT_KEY, &count_str);
  int count = std::atoi(count_str.c_str());
  auto endpoint = EndpointDescriptor::new_endpoint(count, path, "admin");
  txn->Put(ENDPOINTS_COUNT_KEY, std::to_string(count));
  if (!status.ok()) {
    VLOG(WARNING) << "Error creating column family: " << status.ToString();
  }
  _cfh_map.emplace(path, handle);

  folly::dynamic val = folly::dynamic::array();
  status = txn->Put(_db->DefaultColumnFamily(), path, "0");
  if (!status.ok()) {
    VLOG(WARNING) << "Error creating column family: " << status.ToString();
  }
  txn->Commit();
  delete txn;
}
std::vector<std::string> DbManager::get_endpoints() const {
  auto opts = rocksdb::ReadOptions();
  auto iterator = _db->NewIterator(opts);
  std::vector<std::string> res;
  iterator->SeekToFirst();
  while (iterator->Valid()) {
    res.push_back(iterator->key().ToString());
    iterator->Next();
  }
  delete iterator;
  return res;
}
folly::Optional<folly::dynamic> DbManager::get(const std::string path) const {
  using folly::parseJson;
  using google::GLOG_INFO;

  auto readOpts = rocksdb::ReadOptions();
  if (is_endpoint(path)) {
    folly::dynamic ret = folly::dynamic::array();
    auto it = _db->NewIterator(readOpts, _cfh_map.at(path));
    it->SeekToFirst();
    VLOG(GLOG_INFO) << "iterating cf";
    while (it->Valid()) {
      VLOG(GLOG_INFO) << it->key().ToString() << ": " << it->value().ToString();
      std::string value = it->value().ToString();
      folly::dynamic v = parseJson(value);
      ret.push_back(v);
      it->Next();
    }

    return ret;
  }

  std::string value;
  bool found = false;
  bool may_exist = _db->KeyMayExist(readOpts, path, &value, &found);
  if (found) {
    return parseJson(value);
  }
  if (may_exist) {
    auto s = _db->Get(readOpts, path, &value);
    if (s.ok()) {
      return parseJson(value);
    }
  }

  return folly::none;
}
bool DbManager::is_endpoint(const std::string &path) const {
  auto eps = get_endpoints();
  return std::find(eps.begin(), eps.end(), path) != eps.end();
}
folly::Optional<folly::dynamic> DbManager::get_user(const std::string &username) {

  std::string value;
  auto s = _db->NewIterator(rocksdb::ReadOptions(), _cfh_map[USERS_KEY]);
  s->SeekToFirst();
  while (s->Valid()) {
    VLOG(google::GLOG_INFO) << "Iterating over users " << s->key().ToString() << "  --  " << s->value().ToString();
    auto str = s->value();
    auto obj = folly::parseJson(str.ToString());
    if (obj.at("username").asString() == username) {
      VLOG(google::GLOG_INFO) << "hay mas baratos! ";

      return obj;
    }
    s->Next();
  }
  delete s;
  return folly::none;
}
void DbManager::get_all(const std::string &path, std::vector<folly::dynamic> &result) {
  auto s = _db->NewIterator(rocksdb::ReadOptions(), _cfh_map[USERS_KEY]);
  s->SeekToFirst();
  while (s->Valid()) {
    VLOG(google::GLOG_INFO) << "Iterating over users " << s->key().ToString() << "  --  " << s->value().ToString();
    auto str = s->value();
    auto obj = folly::parseJson(str.ToString());
    result.push_back(obj);
    s->Next();
  }
  delete s;

}
int DbManager::get_endpoint_count_and_increment(const std::string path, rocksdb::Transaction *txn) {
  std::string count;
  auto status = txn->GetForUpdate(rocksdb::ReadOptions(), path, &count);
  if (!status.ok()) {
    txn->Rollback();
    VLOG(google::GLOG_WARNING) << "no pude commitear la transaccion: " << status.ToString();
    throw std::runtime_error("could not resolve new id");
  }
  int val = std::atoi(count.c_str());
  count = std::to_string(val + 1);
  txn->Put(path, count);
  return val;
}
void DbManager::perform_database_init_tasks() {

  rocksdb::TransactionDB *db;
  auto opts = rocksdb::Options();
  opts.create_if_missing = true;
  opts.error_if_exists = false;
  opts.create_missing_column_families = true;
  opts.OptimizeForSmallDb();
  auto cfOpts = rocksdb::ColumnFamilyOptions(opts);
  std::vector<rocksdb::ColumnFamilyHandle*> handles;

  std::vector<rocksdb::ColumnFamilyDescriptor> cfDescr = {
      rocksdb::ColumnFamilyDescriptor(rocksdb::kDefaultColumnFamilyName, cfOpts),
      rocksdb::ColumnFamilyDescriptor(ENDPOINTS_COUNT_KEY, cfOpts),
      rocksdb::ColumnFamilyDescriptor(USERS_KEY, cfOpts),
      rocksdb::ColumnFamilyDescriptor(TEST_ENDPOINT_KEY, cfOpts)
  };
  auto txnOpts = rocksdb::TransactionDBOptions();
  auto status = rocksdb::TransactionDB::Open(opts,
                                             txnOpts,
                                             RestDbConfiguration::get_instance()->getDb_path(),
                                             cfDescr,
                                             &handles,
                                             &db);
  //_db.reset(db);
  if (!status.ok()) {
    VLOG(google::GLOG_ERROR) << status.ToString();
  }
  auto writeOpts = rocksdb::WriteOptions();

  status = db->Put(writeOpts, ENDPOINTS_COUNT_KEY, "0");
  if (!status.ok()) {
    VLOG(google::GLOG_ERROR) << status.ToString();
  }
  for (auto &h: handles) {
    db->DestroyColumnFamilyHandle(h);
  }
  delete db;
}

bool DbManager::is_initialized() {
  auto opts = rocksdb::Options();
  opts.create_if_missing = false;
  opts.error_if_exists = false;
  auto cfOpts = rocksdb::ColumnFamilyOptions(opts);

  cfOpts.OptimizeForSmallDb();

  std::vector<std::string> cfNames;
  auto status = rocksdb::DB::ListColumnFamilies(rocksdb::DBOptions(opts), RestDbConfiguration::get_instance()->getDb_path(), &cfNames);
  if (!status.ok()) {
    VLOG(google::GLOG_WARNING) << status.ToString();
    return false;
  }
  if (std::find(cfNames.begin(), cfNames.end(), ENDPOINTS_KEY) == cfNames.end()) {
    return false;
  }
  if (std::find(cfNames.begin(), cfNames.end(), USERS_KEY) == cfNames.end()) {
    return false;
  }
  if (std::find(cfNames.begin(), cfNames.end(), TEST_ENDPOINT_KEY) == cfNames.end()) {
    return false;
  }
  return true;

}
}
