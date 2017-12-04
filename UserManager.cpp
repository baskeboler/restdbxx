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
std::unique_ptr<AccessToken> UserManager::get_access_token(const string &username) {
  auto token = createToken(username);
  auto db = DbManager::get_instance();
  auto user = db->get_user(username);
  auto json = token->toDynamic();
  string key = "/__tokens/" + token->token;
  db->raw_save(key, json, "/__tokens");
  return token;
}
std::unique_ptr<AccessToken> UserManager::createToken(const string &username) {
  auto res = std::make_unique<AccessToken>();
  res->username = username;
  res->valid_until = boost::posix_time::second_clock::universal_time() + boost::posix_time::minutes(5);
  char randomData[16];
  folly::Random::secureRandom(randomData, 16);
  res->token = proxygen::base64Encode(folly::range(randomData, randomData + 16));
//      folly::
  //boost::local_time::local_date_time v(boost::posix_time::;
  return res;
}
bool UserManager::user_exists(string username) const {
  auto db = DbManager::get_instance();
  auto maybe_user = db->get_user(username);
  return maybe_user.hasValue();
}
bool UserManager::validate_access_token(const string &username, const string &token) {
  string tokenPath = "/__tokens/" + token;
  auto db = DbManager::get_instance();
  auto tokenObj = db->raw_get(tokenPath, "/__tokens");
  if (tokenObj) {
    if (tokenObj->isObject()) {
      try {
        auto token_obj = AccessToken::fromDynamic(tokenObj.value());
        bool expired = token_obj->valid_until < boost::posix_time::second_clock::universal_time();
        if (expired) {
          VLOG(google::GLOG_INFO) << "token has expired";
          return false;
        }
        return token_obj->username == username;
      } catch (std::domain_error &e) {
        VLOG(google::GLOG_INFO) << "could not get token: " << e.what();
        return false;
      }
    }
    return false;
  }
  return false;
}
std::unique_ptr<User> UserManager::create_user(const std::string &username, const std::string &password) {
  auto user = std::make_unique<User>();
  user->setUsername(username);
  folly::StringPiece bytes = password;
  user->setPassword(md5Encode(bytes));
  user->setIs_active(true);

  auto db = DbManager::get_instance();
  auto json = user->toDynamic();
  std::string key = "/__users/" + user->getUsername();
  db->raw_save(key, json, "/__users");
  return user;
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
folly::dynamic AccessToken::toDynamic() {
  return folly::dynamic::object("username", username)
      ("token", token)
      ("valid_until", boost::posix_time::to_iso_string(valid_until));
}
}