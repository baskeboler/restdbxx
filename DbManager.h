//
// Created by victor on 23/10/17.
//

#ifndef RESTDBXX_DBMANAGER_H
#define RESTDBXX_DBMANAGER_H

#include  <string>
#include <vector>
#include <memory>
#include <boost/algorithm/string.hpp>
//#include <rocksdb/utilities/optimistic_transaction_db.h>
#include  <folly/ExceptionWrapper.h>
#include <folly/dynamic.h>
#include <folly/Optional.h>
#include <rocksdb/utilities/transaction_db.h>
#include <folly/json.h>
#include <glog/logging.h>
#include <glog/stl_logging.h>

namespace restdbxx {

class DbManager {
 public:;
  DbManager();

  bool path_exists(std::string path);
  bool can_post(const std::string path);

  //folly::dynamic to_deep_object(std::vector<std::string> &path, const folly::dynamic &unwrapped);

  //void deep_merge(folly::dynamic &dest, folly::dynamic &merge_obj);

  void raw_save(const std::string &key,
                folly::dynamic &data,
                const std::string &cf_name = rocksdb::kDefaultColumnFamilyName);

  folly::Optional<folly::dynamic> raw_get(const std::string &key,
                                          const std::string &cf_name = rocksdb::kDefaultColumnFamilyName);
  /**
   * @brief will add ID property to data
   * @param path path to post to
   * @param data json object
   */
  void post(std::string path, folly::dynamic &data);
  void put(std::string path, const folly::dynamic &data);

  folly::Optional<folly::dynamic> get(const std::string path) const;
  void remove(const std::string path);

  void add_endpoint(const std::string &path);
  folly::dynamic get_endpoint(const std::string &path) const;

  std::vector<std::string> get_endpoints() const;
  bool is_endpoint(const std::string& path) const;

  static std::shared_ptr<DbManager> get_instance();
  virtual ~DbManager();

  /*void save_user(folly::dynamic &userJson) {
    if (!get_user(userJson["password"].asString())) this->post(USERS_CF, userJson);
  }*/

  folly::Optional<folly::dynamic> get_user(const std::string &username);

  void get_all(const std::string &path, std::vector<folly::dynamic> &result);
 private:
  std::unique_ptr<rocksdb::TransactionDB> _db;
  std::vector<std::string> get_path_parts(std::string path) const;
  std::vector<rocksdb::ColumnFamilyHandle *> _handles;
  std::map<std::string, rocksdb::ColumnFamilyHandle *> _cfh_map;
  int get_endpoint_count_and_increment(const std::string pTransaction, rocksdb::Transaction *pTransaction1);
  void perform_database_init_tasks();
  bool is_initialized();
};

}

#endif //RESTDBXX_DBMANAGER_H
