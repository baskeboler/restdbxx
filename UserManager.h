//
// Created by victor on 4/11/17.
//

#ifndef RESTDBXX_USERMANAGER_H
#define RESTDBXX_USERMANAGER_H

#include "User.h"
#include "DbManager.h"
#include <proxygen/lib/utils/CryptUtil.h>
#include <memory>
using std::shared_ptr;
using proxygen::md5Encode;

namespace restdbxx {
class UserManager {

 public:
  static shared_ptr<UserManager> get_instance();

  bool authenticate(string username, string password) const;


};

}

#endif //RESTDBXX_USERMANAGER_H
