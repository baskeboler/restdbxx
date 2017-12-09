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
static folly::Singleton<DbManager, DbManagerSingletonTag> the_instance;

constexpr const char ENDPOINTS_KEY[] = "/__endpoints";
constexpr const char ENDPOINTS_COUNT_KEY[] = "/__endpoints/count";
constexpr const char USERS_KEY[] = "/__users";
constexpr const char TEST_ENDPOINT_KEY[] = "/test_endpoint";
constexpr const char TOKENS_KEY[] = "/__tokens";
constexpr const char ALL_OBJECTS_KEY[] = "/___all_objects___";
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
  const std::string &db_path = RestDbConfiguration::get_instance()->getDb_path();
  auto opts = rocksdb::Options();
  //auto dbOpts = rocksdb::DBOptions();
  opts.create_if_missing = true;
  opts.create_missing_column_families = true;
  opts.OptimizeForSmallDb();
  //dbOpts.
  std::vector<std::string> cfs;

  auto status = rocksdb::DB::ListColumnFamilies(opts, db_path, &cfs);
  if (!status.ok()) {
    VLOG(GLOG_ERROR) << status.ToString();
  }

  std::vector<rocksdb::ColumnFamilyDescriptor> descrs;
  const auto cfOpts = rocksdb::ColumnFamilyOptions(opts);
  for (const std::string &n: cfs) {
    descrs.emplace_back(n, cfOpts);
  }
  rocksdb::TransactionDB *db;
  std::vector<rocksdb::ColumnFamilyHandle *> handles;
  handles.clear();
  status = rocksdb::TransactionDB::Open(opts,
                                        rocksdb::TransactionDBOptions(),
                                        db_path,
                                        const_cast<const std::vector<rocksdb::ColumnFamilyDescriptor> &>(descrs),
                                        &handles,
                                        &db);
  if (!status.ok()) {
    VLOG(google::GLOG_ERROR) << status.ToString();
  }
  _db.reset(db);
  VLOG(GLOG_INFO) << "Listing column families";
  for (auto handle: handles) {
    VLOG(GLOG_INFO) << handle->GetName();
    std::string hName(handle->GetName());
    _cfh_map.insert(std::make_pair<std::string, rocksdb::ColumnFamilyHandle *>(move(hName), std::move(handle)));
  }
  VLOG(GLOG_INFO) << "End of column families";

  if (!is_endpoint(TEST_ENDPOINT_KEY)) {
    _db->Put(rocksdb::WriteOptions(), ENDPOINTS_COUNT_KEY, "0");
    VLOG(GLOG_INFO) << "Test endpoint inexistent, creating.";
    add_endpoint(TEST_ENDPOINT_KEY);
    add_endpoint(USERS_KEY);
    add_endpoint(TOKENS_KEY);
  }

  cleanTokens();
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
    batch.Put(_cfh_map.at(ALL_OBJECTS_KEY), new_path, new_val);
    //batch.Put(new_path, new_val);
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
      std::string val = folly::toPrettyJson(data);
      auto status = trx->Put(_cfh_map[path], new_key, val);
      if (status.ok())
        status = trx->Put(_cfh_map.at(ALL_OBJECTS_KEY), new_key, val);
      if (status.ok())
        status = trx->Commit();
      if (!status.ok()) {
        trx->Rollback();
        VLOG(google::GLOG_WARNING) << "no pude commitear la transaccion: " << status.ToString();
        return;
      }
    } else {
      _db->Put(rocksdb::WriteOptions(), _cfh_map.at(ALL_OBJECTS_KEY), path, folly::toJson(data));
    }
  }
}
void DbManager::remove(const std::string path) {
  //folly::make_exception_wrapper()
  if (path_exists(path)) {
    auto parts = get_path_parts(path);
    //  auto aux = &_root;
    rocksdb::TransactionOptions txnOpts;
    rocksdb::WriteOptions writeOpts;

//    auto txn = _db->BeginTransaction(writeOpts, txnOpts);
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
  for (auto h: _cfh_map) {
    auto s = _db->DestroyColumnFamilyHandle(h.second);
    if (!s.ok()) LOG(ERROR) << s.ToString();
  }
  _db->Flush(rocksdb::FlushOptions());
  //_db->reset();
}
void DbManager::add_endpoint(const std::string &path) {
  using rocksdb::DB;
  using google::WARNING;
  VLOG(google::GLOG_INFO) << "Creating endpoint " << path;
  std::string count_str;
  rocksdb::ColumnFamilyHandle *handle;

  auto cfOpts = rocksdb::ColumnFamilyOptions();
  cfOpts.OptimizeForSmallDb();
  auto status = _db->GetBaseDB()->CreateColumnFamily(cfOpts, path, &handle);
  if (!status.ok()) {
    VLOG(google::GLOG_ERROR) << status.ToString();
  }
  auto txn = _db->BeginTransaction(rocksdb::WriteOptions());
  status = txn->Get(rocksdb::ReadOptions(), ENDPOINTS_COUNT_KEY, &count_str);
  if (!status.ok()) {
    VLOG(google::GLOG_ERROR) << status.ToString();
  }
  int count = std::atoi(count_str.c_str());
  auto endpoint = EndpointDescriptor::new_endpoint(count, path, "admin");
  std::string endpoint_descr_key = ENDPOINTS_KEY + path;
  status = txn->Put(_cfh_map[ENDPOINTS_KEY], endpoint_descr_key, folly::toPrettyJson(endpoint->getDynamic()));
  if (!status.ok()) {
    VLOG(google::GLOG_ERROR) << status.ToString();
  }
  status = txn->Put(ENDPOINTS_COUNT_KEY, std::to_string(count));
  if (!status.ok()) {
    VLOG(WARNING) << "Error creating column family: " << status.ToString();
  }
  _cfh_map[path] = handle;

  folly::dynamic val = folly::dynamic::array();
  status = txn->Put(_db->DefaultColumnFamily(), path, "0");
  if (!status.ok()) {
    VLOG(WARNING) << "Error creating column family: " << status.ToString();
  }
  status = txn->Commit();
  if (!status.ok()) {
    VLOG(google::GLOG_ERROR) << status.ToString();
  }
  delete txn;
}

std::vector<std::string> DbManager::get_endpoints() const {
  auto opts = rocksdb::ReadOptions();
  auto iterator = _db->GetBaseDB()->NewIterator(opts);
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
//    VLOG(GLOG_INFO) << "iterating cf";
    while (it->Valid()) {
      //    VLOG(GLOG_INFO) << it->key().ToString() << ": " << it->value().ToString();
      std::string value = it->value().ToString();
      folly::dynamic v = parseJson(value);
      ret.push_back(v);
      it->Next();
    }
    delete it;
    return ret;
  }

  std::string value;
  auto s = _db->Get(readOpts, _cfh_map.at(ALL_OBJECTS_KEY), path, &value);
  if (s.ok()) {
    return parseJson(value);
  }

  return folly::none;
}
bool DbManager::is_endpoint(const std::string &path) const {
  auto eps = get_endpoints();
  return std::find(eps.begin(), eps.end(), path) != eps.end();
}
folly::Optional<folly::dynamic> DbManager::get_user(const std::string &username) {

  std::string value;
  std::string key = std::string(USERS_KEY) + "/" + username;
  auto s = _db->Get(rocksdb::ReadOptions(), _cfh_map.at(USERS_KEY), key, &value);
  if (s.ok())
    return folly::parseJson(value);
  return folly::none;
}
void DbManager::get_all(const std::string &path, std::vector<folly::dynamic> &result) {
  if (_cfh_map.find(path) == _cfh_map.end() || _cfh_map[path] == nullptr) {
    throw DbManagerException("column family handle unavailable");
  }
  auto s = _db->GetBaseDB()->NewIterator(rocksdb::ReadOptions(), _cfh_map[path]);
  s->SeekToFirst();
  while (s->Valid()) {
    VLOG(google::GLOG_INFO) << "Iterating over users " << s->key().ToString() << "  --  " << s->value().ToString();
    auto str = s->value();
    auto obj = folly::parseJson(str.ToString());
    result.emplace_back(obj);
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
  std::vector<rocksdb::ColumnFamilyHandle *> handles;

  std::vector<rocksdb::ColumnFamilyDescriptor> cfDescr = {
      rocksdb::ColumnFamilyDescriptor(rocksdb::kDefaultColumnFamilyName, cfOpts),
      rocksdb::ColumnFamilyDescriptor(ALL_OBJECTS_KEY, cfOpts),
      rocksdb::ColumnFamilyDescriptor(ENDPOINTS_KEY, cfOpts),
      rocksdb::ColumnFamilyDescriptor(ENDPOINTS_COUNT_KEY, cfOpts),
      rocksdb::ColumnFamilyDescriptor(USERS_KEY, cfOpts),
      rocksdb::ColumnFamilyDescriptor(TEST_ENDPOINT_KEY, cfOpts),
      rocksdb::ColumnFamilyDescriptor(TOKENS_KEY, cfOpts),
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
  auto status = rocksdb::DB::ListColumnFamilies(rocksdb::DBOptions(opts),
                                                RestDbConfiguration::get_instance()->getDb_path(),
                                                &cfNames);
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
  if (std::find(cfNames.begin(), cfNames.end(), ALL_OBJECTS_KEY) == cfNames.end()) {
    return false;
  }
  if (std::find(cfNames.begin(), cfNames.end(), TEST_ENDPOINT_KEY) == cfNames.end()) {
    return false;
  }
  if (std::find(cfNames.begin(), cfNames.end(), TOKENS_KEY) == cfNames.end()) {
    return false;
  }
  return true;

}
folly::dynamic DbManager::get_endpoint(const std::string &path) const {

  rocksdb::Status status;
  std::string key = ENDPOINTS_KEY + path;
  std::string value;
  status = _db->Get(rocksdb::ReadOptions(), _cfh_map.at(ENDPOINTS_KEY), key, &value);
  if (status.ok()) {
    return folly::parseJson(value);
  }
  return nullptr;
}
void DbManager::raw_save(const std::string &key, folly::dynamic &data, const std::string &cf_name) {

  std::string val = folly::toPrettyJson(data);
  auto txn = _db->BeginTransaction(rocksdb::WriteOptions());
  auto s = txn->Put(_cfh_map.at(cf_name), key, val);
  if (s.ok())
    s = txn->Put(_cfh_map.at(ALL_OBJECTS_KEY), key, val);
  if (s.ok())
    s = txn->Commit();
  if (!s.ok()) {
    VLOG(google::GLOG_INFO) << "error saving token: " << s.ToString();
    txn->Rollback();
  }
  delete txn;
}
folly::Optional<folly::dynamic> DbManager::raw_get(const std::string &key, const std::string &cf_name) {
  std::string value;
  auto s = _db->Get(rocksdb::ReadOptions(), _cfh_map.at(cf_name), key, &value);
  if (!s.ok()) {
    VLOG(google::GLOG_INFO) << s.ToString();
    return folly::none;
  }
  return folly::parseJson(value);
}
void DbManager::cleanTokens() {
  VLOG(google::GLOG_INFO) << "access token cleanup";
  auto i = _db->NewIterator(rocksdb::ReadOptions(), _cfh_map.at(TOKENS_KEY));
  i->SeekToFirst();
  std::vector<std::string> expired;
  while (i->Valid()) {
    auto val = i->value();
    auto key = i->key();
    i->Next();
    auto json = folly::parseJson(val.ToString());
    boost::posix_time::ptime timestamp = boost::posix_time::from_iso_string(json.at("valid_until").asString());
    bool is_valid = timestamp > boost::posix_time::second_clock::universal_time();
    if (!is_valid) expired.push_back(key.ToString());
  }
  delete i;
  auto s = rocksdb::Status::OK();
  auto opts = rocksdb::WriteOptions();
  opts.sync = true;

  for (auto &k: expired) {
    VLOG(google::GLOG_INFO) << "Deleting expired access token " << k;
    s = _db->GetBaseDB()->Delete(opts, _cfh_map.at(TOKENS_KEY), k);
    if (!s.ok()) {
      VLOG(google::GLOG_INFO) << s.ToString();
    }
  }
}
folly::Optional<folly::dynamic> DbManager::raw_get(const std::string &key) {
  return raw_get(key, ALL_OBJECTS_KEY);
}
void DbManager::delete_endpoint(const std::string &path) {

  if (!is_endpoint(path))
    throw DbManagerException("path is not an endpoint");
  rocksdb::TransactionOptions tOpts;
  rocksdb::WriteOptions wOpts;
  wOpts.sync = true;

  rocksdb::ReadOptions rOpts;
//  rOpts.managed
  rocksdb::Status s;
  auto txn = _db->BeginTransaction(wOpts, tOpts);

  if (_cfh_map.find(path) != _cfh_map.end() && _cfh_map[path] != nullptr) {
    auto i = _db->NewIterator(rOpts, _cfh_map[path]);

    i->SeekToFirst();
//  std::vector<std::string> paths;
    while (i->Valid()) {
      VLOG(google::GLOG_INFO) << "Deleting key " << i->key().ToString();
      txn->Delete(i->key());
      if (!s.ok()) {
        throw DbManagerException(s.ToString());
      }
      i->Next();
    }
    delete i;
  }

  VLOG(google::GLOG_INFO) << "Deleting path " << path;
  s = txn->Delete(path);
  if (!s.ok()) {
    throw DbManagerException(s.ToString());
  }

  VLOG(google::GLOG_INFO) << "Deleting endpoint descriptor ";
  std::string descr_key = ENDPOINTS_KEY + path;
  s = txn->Delete(_cfh_map.at(ENDPOINTS_KEY), descr_key);
  if (!s.ok()) {
    throw DbManagerException(s.ToString());
  }

  VLOG(google::GLOG_INFO) << "Commiting transaction";
  txn->Commit();
  delete txn;

  VLOG(google::GLOG_INFO) << "Erasing column family from cf map";
  auto i = _cfh_map.find(path);
  rocksdb::ColumnFamilyHandle *h{nullptr};
  if (i != _cfh_map.end()) {
    h = i->second;
    _cfh_map.erase(i);
  }
  VLOG(google::GLOG_INFO) << "Destroying column family for " << path;
  s = _db->GetBaseDB()->DropColumnFamily(h);
  if (!s.ok()) {
    throw DbManagerException(s.ToString());
  }

}
}