//
// Created by victor on 4/11/17.
//

#ifndef RESTDBXX_VALIDATIONS_H
#define RESTDBXX_VALIDATIONS_H
#include <string>
#include <boost/algorithm/string.hpp>
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

  static std::string get_endpoint_from_path(const std::string &path) {
    std::vector<std::string> parts;
    boost::algorithm::split(parts, path, boost::is_any_of("/"), boost::token_compress_on);
    if (parts.empty()) {
      return "/";
    } else {
      return "/" + parts[0];
    }
  }
};

}

#endif //RESTDBXX_VALIDATIONS_H
