//
// Created by victor on 23/10/17.
//

#ifndef RESTDBXX_DBMANAGER_H
#define RESTDBXX_DBMANAGER_H
#include <string>
#include <boost/algorithm/string.hpp>
#include <vector>
#include <folly/dynamic.h>
#include <folly/Optional.h>
#include <folly/json.h>
#include <folly/ExceptionWrapper.h>
#ifndef _NDEBUG
#include <glog/logging.h>
#include <glog/stl_logging.h>
#endif
namespace restdbxx {
class DbManager {
 public:
  DbManager();

  folly::Optional<folly::dynamic> get_path(const std::string &path);

  bool path_exists(const std::string& path);

  folly::dynamic to_deep_object(std::vector<std::string> &path, const folly::dynamic& unwrapped);

  void deep_merge(folly::dynamic &dest, folly::dynamic &merge_obj);
  void post(const std::string& path, const folly::dynamic &data);
  void put(const std::string& path, const folly::dynamic& data);

  void remove(const std::string& path);
  void debug_root() {

    LOG(INFO) << folly::toPrettyJson(_root);
  }

  static std::shared_ptr<DbManager> get_instance();
 private:
  folly::dynamic _root;
  std::vector<std::string> get_path_parts(const std::string &path) const;
};

}

#endif //RESTDBXX_DBMANAGER_H
