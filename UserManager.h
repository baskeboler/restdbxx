//
// Created by victor on 4/11/17.
//

#ifndef RESTDBXX_USERMANAGER_H
#define RESTDBXX_USERMANAGER_H

#include "User.h"
#include "DbManager.h"
#include <proxygen/lib/utils/CryptUtil.h>
#include <memory>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <folly/Random.h>

using std::shared_ptr;
using proxygen::md5Encode;

namespace restdbxx {

struct AccessToken {
  std::string username;
  std::string token;
  boost::posix_time::ptime valid_until;

  folly::dynamic toDynamic() {
    return folly::dynamic::object("username", username)
        ("token", token)
        ("valid_until", boost::posix_time::to_iso_string(valid_until));
  }

  static std::unique_ptr<AccessToken> fromDynamic(folly::dynamic &json);
};

class UserManager {

 public:
  static shared_ptr<UserManager> get_instance();

  std::unique_ptr<User> create_user(const std::string &username, const std::string &password) {
    auto user = std::make_unique<User>();
    user->setUsername(username);
    folly::StringPiece bytes = password;
    user->setPassword(md5Encode(bytes));
    user->setIs_active(true);

    auto db = DbManager::get_instance();
    auto json = user->toDynamic();
    db->post("/__users", json);
    return user;
  }
  bool authenticate(string username, string password) const;

  bool user_exists(string username) const {
    auto db = DbManager::get_instance();
    auto maybe_user = db->get_user(username);
    return maybe_user.hasValue();
  }

  std::unique_ptr<AccessToken> get_access_token(const string &username) {
    auto token = createToken(username);
    auto db = DbManager::get_instance();
    auto user = db->get_user(username);
    auto json = token->toDynamic();
    string key = "/__tokens/" + token->token;
    db->raw_save(key, json, "/__tokens");
    return token;
  }
  std::unique_ptr<AccessToken> createToken(const string &username) {
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

  bool validate_access_token(const string &username, const string &token) {
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
};

}

#endif //RESTDBXX_USERMANAGER_H
