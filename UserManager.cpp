//
// Created by victor on 4/11/17.
//

#include <folly/Singleton.h>
#include "UserManager.h"

namespace restdbxx {
namespace {
struct UserManagerTag {};
folly::Singleton<UserManager, UserManagerTag> the_instance;
}

/**
 * @brief
 * @return
 */
shared_ptr<UserManager> UserManager::get_instance() {
  return the_instance.try_get();
}

/**
 * @brief
 * @param username
 * @param password
 * @return
 */
bool UserManager::authenticate(string username, string password) const {
  VLOG(google::GLOG_INFO) << "Authenticating user " << username << " with pass " << password;
  auto db = DbManager::get_instance();
  auto res = db->get_user(username);
  if (res) {
    auto &json = res.value();
    folly::StringPiece byteRange = password;

    string md5Password = md5Encode(byteRange);
    return md5Password == json.at("password").asString();
  }
  return false;
}
}