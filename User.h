//
// Created by victor on 4/11/17.
//

#ifndef RESTDBXX_USER_H
#define RESTDBXX_USER_H
#include <string>
#include <boost/date_time/local_time/local_date_time.hpp>
#include <folly/dynamic.h>
using std::string;
using namespace boost::date_time;

namespace restdbxx {
class User {
 private:
  string username;
  string password;
  bool is_active;

 public:
  const string &getUsername() const;
  void setUsername(const string &username);
  const string &getPassword() const;
  void setPassword(const string &password);
  bool isIs_active() const;
  void setIs_active(bool is_active);

  folly::dynamic toDynamic() {
    return folly::dynamic::object("username", username)
        ("password", password)
        ("is_active", is_active);
  }
};


}
#endif //RESTDBXX_USER_H
