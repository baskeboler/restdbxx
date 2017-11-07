//
// Created by victor on 4/11/17.
//

#ifndef RESTDBXX_VALIDATIONS_H
#define RESTDBXX_VALIDATIONS_H
#include <string>

namespace restdbxx {
class Validations {
 public:
  /**
   * @brief
   * @param email
   * @return
   */
  static bool is_valid_email(const std::string &email);

  /**
   * @brief
   * @param path
   * @return
   */
  static bool is_valid_path(const std::string &path);

  /**
   * @brief
   * @param path
   */
  static void sanitize_path(std::string &path);
};

}

#endif //RESTDBXX_VALIDATIONS_H
