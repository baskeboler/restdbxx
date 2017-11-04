//
// Created by victor on 4/11/17.
//

#ifndef RESTDBXX_VALIDATIONS_H
#define RESTDBXX_VALIDATIONS_H
#include <string>

namespace restdbxx {
class Validations {
 public:
  static bool is_valid_email(const std::string &email);
  static bool is_valid_path(const std::string &path);
  static void sanitize_path(std::string &path);
};

}

#endif //RESTDBXX_VALIDATIONS_H
