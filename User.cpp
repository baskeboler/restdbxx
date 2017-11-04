//
// Created by victor on 4/11/17.
//

#include "User.h"

namespace restdbxx {
const string &User::getUsername() const {
  return username;
}
void User::setUsername(const string &username) {
  User::username = username;
}
const string &User::getPassword() const {
  return password;
}
void User::setPassword(const string &password) {
  User::password = password;
}
bool User::isIs_active() const {
  return is_active;
}
void User::setIs_active(bool is_active) {
  User::is_active = is_active;
}

}
