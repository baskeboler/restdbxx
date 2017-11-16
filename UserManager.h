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

  folly::dynamic toDynamic();

  static std::unique_ptr<AccessToken> fromDynamic(folly::dynamic &json);
};

class UserManager {

 public:
  static shared_ptr<UserManager> get_instance();

  std::unique_ptr<User> create_user(const std::string &username, const std::string &password);
  bool authenticate(string username, string password) const;

  bool user_exists(string username) const;

  std::unique_ptr<AccessToken> get_access_token(const string &username);
  std::unique_ptr<AccessToken> createToken(const string &username);

  bool validate_access_token(const string &username, const string &token);
};

}

#endif //RESTDBXX_USERMANAGER_H
