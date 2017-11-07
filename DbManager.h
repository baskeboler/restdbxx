//
// Created by victor on 23/10/17.
//

#ifndef RESTDBXX_DBMANAGER_H
#define RESTDBXX_DBMANAGER_H
#include <string>
#include <vector>
#include <memory>
#include <boost/algorithm/string.hpp>
//#include <rocksdb/utilities/optimistic_transaction_db.h>
#include <folly/ExceptionWrapper.h>
#include <folly/dynamic.h>
#include <folly/Optional.h>
#include <rocksdb/db.h>
#include <folly/json.h>
#include <glog/logging.h>
#include <glog/stl_logging.h>

namespace restdbxx {

static const char *USERS_CF = "/__users";

class DbManager {
 public:;
  DbManager();

  bool path_exists(std::string path);
  bool can_post(const std::string path);

  //folly::dynamic to_deep_object(std::vector<std::string> &path, const folly::dynamic &unwrapped);

  //void deep_merge(folly::dynamic &dest, folly::dynamic &merge_obj);

  /**
   * @brief will add ID property to data
   * @param path path to post to
   * @param data json object
   */
  void post(const std::string path, folly::dynamic &data);
  void put(const std::string path, const folly::dynamic &data);

  folly::Optional<folly::dynamic> get(const std::string path) const;
  void remove(const std::string path);

  void add_endpoint(const std::string path);


  std::vector<std::string> get_endpoints() const;
  bool is_endpoint(const std::string& path) const;

  static std::shared_ptr<DbManager> get_instance();
  virtual ~DbManager();

  void save_user(folly::dynamic &userJson) {
    if (!get_user(userJson["password"].asString())) this->post(USERS_CF, userJson);
  }

  folly::Optional<folly::dynamic> get_user(const std::string &username);

  void get_all(const std::string &path, std::vector<folly::dynamic> &result);
 private:
  rocksdb::DB *_db;
  folly::dynamic _root;
  std::vector<std::string> get_path_parts(std::string path) const;
  std::vector<rocksdb::ColumnFamilyHandle *> handles;
  std::map<std::string, rocksdb::ColumnFamilyHandle *> cfh_map;
};

}

#endif //RESTDBXX_DBMANAGER_H
