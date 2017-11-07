//
// Created by victor on 23/10/17.
//

#include "DbManager.h"
#include "RestDbConfiguration.h"
#include <folly/Singleton.h>
namespace restdbxx {
namespace {
struct DbManagerSingletonTag {};
folly::Singleton<DbManager, DbManagerSingletonTag> the_instance;
}

DbManager::DbManager() : _root(folly::dynamic::object()) {

  //_root = folly::dynamic::object();
  using google::GLOG_INFO;
  using google::GLOG_ERROR;
  //_root.
  VLOG(GLOG_INFO) << "Initializing DbManager";
  const auto &db_path = RestDbConfiguration::get_instance()->getDb_path();
  auto opts = rocksdb::Options();
  //auto dbOpts = rocksdb::DBOptions();
  opts.create_if_missing = true;
  //dbOpts.
  std::vector<std::string> cfs;

  rocksdb::DB *aux;
  auto status = rocksdb::DB::Open(opts, db_path, &aux);
  if (!status.ok()) {
    VLOG(GLOG_ERROR) << status.ToString();
  }
  delete aux;

  status = rocksdb::DB::ListColumnFamilies(opts, db_path, &cfs);
  if (!status.ok()) {
    VLOG(GLOG_ERROR) << status.ToString();
  }

  std::vector<rocksdb::ColumnFamilyDescriptor> descrs;
  for (const auto &n: cfs) {
    descrs.emplace_back(n, rocksdb::ColumnFamilyOptions());
  }
  status = rocksdb::DB::Open(opts, db_path, descrs, &handles, &_db);
  if (!status.ok()) {
    LOG(ERROR) << status.ToString();
  }
  VLOG(GLOG_INFO) << "Listing column families";
  for (auto handle: handles) {
    VLOG(GLOG_INFO) << handle->GetName();
    cfh_map.emplace(handle->GetName(), handle);
  }
  VLOG(GLOG_INFO) << "End of column families";

  if (!is_endpoint("/test_endpoint")) {
    VLOG(GLOG_INFO) << "Test endpoint inexistent, creating.";
    add_endpoint("/test_endpoint");
    add_endpoint(USERS_CF);
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
    batch.Put(cfh_map.at(path), new_path, new_val);
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

std::shared_ptr<DbManager> DbManager::get_instance() {
  return the_instance.try_get();
}
void DbManager::put(const std::string path, const folly::dynamic &data) {
  LOG(INFO) << "Request to put to: " << path;
  if (path_exists(path)) {
    auto tokens = get_path_parts(path);
    auto aux = &_root;
    for (auto part = tokens.begin(); part != tokens.end(); ++part) {
      if (std::distance(part, tokens.end()) == 1) {
        //(*aux)[*part] = data;
        aux->insert(*part, data);
        return;
      }

      aux = &aux->at(*part);

    }
  }
}
void DbManager::remove(const std::string path) {
  //folly::make_exception_wrapper()
  if (path_exists(path)) {
    auto parts = get_path_parts(path);
    auto aux = &_root;

    for (auto part = parts.begin(); part != parts.end(); ++part) {
      if (std::distance(part, parts.end()) == 1) {
        //last part
        aux->erase(*part);
        return;
      }
      if (aux->isObject()) {
        aux = &aux->at(*part);
      } else {
        LOG(ERROR) << "uh oh";
      }
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
  for (auto h: handles) {
    auto s = _db->DestroyColumnFamilyHandle(h);
    if (!s.ok()) LOG(ERROR) << s.ToString();
  }
  delete _db;
}
void DbManager::add_endpoint(const std::string path) {
  using rocksdb::DB;
  using google::WARNING;

  rocksdb::ColumnFamilyHandle *handle;

  auto cfOpts = rocksdb::ColumnFamilyOptions();
  auto status = _db->CreateColumnFamily(cfOpts, path, &handle);
  if (!status.ok()) {
    VLOG(WARNING) << "Error creating column family: " << status.ToString();
  }
  cfh_map.emplace(path, handle);
  auto writeOpts = rocksdb::WriteOptions();

  folly::dynamic val = folly::dynamic::array();
  status = _db->Put(writeOpts, _db->DefaultColumnFamily(), path, "0");
  if (!status.ok()) {
    VLOG(WARNING) << "Error creating column family: " << status.ToString();
  }
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
    auto it = _db->NewIterator(readOpts, cfh_map.at(path));
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
  //auto j = folly::dynamic::object();
  std::string value;
  auto s = _db->NewIterator(rocksdb::ReadOptions(), cfh_map[USERS_CF]);
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
  auto s = _db->NewIterator(rocksdb::ReadOptions(), cfh_map[USERS_CF]);
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
}
