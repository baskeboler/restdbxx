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
std::unique_ptr<AccessToken> AccessToken::fromDynamic(folly::dynamic &json) {
  auto res = new AccessToken;
  if (!json.isObject())
    BOOST_THROW_EXCEPTION(std::domain_error("invalid access token object"));
  if (!json.at("username").isString())
    BOOST_THROW_EXCEPTION(std::domain_error("access token without username"));
  if (!json.at("token").isString())
    BOOST_THROW_EXCEPTION(std::domain_error("access token without token string"));
  if (!json.at("valid_until").isString())
    BOOST_THROW_EXCEPTION(std::domain_error("access token without username"));
  res->username = string(json.at("username").c_str());
  res->token = string(json.at("token").c_str());
  res->valid_until = boost::posix_time::from_iso_string(string(json.at("valid_until").c_str()));
  return std::unique_ptr<AccessToken>(res);
}
}