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
#include <boost/date_time/posix_time/posix_time.hpp>
#include <stdexcept>

namespace restdbxx {

/**
 *
 */
class DbManagerException : public std::runtime_error {
 public:
  DbManagerException(const std::string &__arg) : std::runtime_error(__arg) {

  }

  DbManagerException() : DbManagerException("DbManager Exception") {}
  virtual ~DbManagerException() = default;
};

/**
 *
 */
class DbManager {
 public:;
  DbManager();

  bool path_exists(std::string path);
  bool can_post(const std::string path);

  void raw_save(const std::string &key,
                folly::dynamic &data,
                const std::string &cf_name = rocksdb::kDefaultColumnFamilyName);

  folly::Optional<folly::dynamic> raw_get(const std::string &key,
                                          const std::string &cf_name);

  folly::Optional<folly::dynamic> raw_get(const std::string &key);
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
  void delete_endpoint(const std::string &path);
  std::vector<std::string> get_endpoints() const;
  bool is_endpoint(const std::string &path) const;

  static std::shared_ptr<DbManager> get_instance();

  virtual ~DbManager();

  folly::Optional<folly::dynamic> get_user(const std::string &username);

  void get_all(const std::string &path, std::vector<folly::dynamic> &result);
 private:
  std::unique_ptr<rocksdb::TransactionDB> _db;
  std::vector<std::string> get_path_parts(std::string path) const;
  std::map<std::string, rocksdb::ColumnFamilyHandle *> _cfh_map;
  int get_endpoint_count_and_increment(
      const std::string pTransaction,
      rocksdb::Transaction *pTransaction1);
  void perform_database_init_tasks();
  bool is_initialized();

  void cleanTokens();
};

}

#endif //RESTDBXX_DBMANAGER_H
