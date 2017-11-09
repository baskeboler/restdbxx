//
// Created by victor on 4/11/17.
//

#include <boost/spirit/include/classic.hpp>
#include <boost/spirit/include/qi_lit.hpp>
#include "Validations.h"
#include <boost/algorithm/string.hpp>
namespace restdbxx {

bool Validations::is_valid_path(const std::string &path) {
  using namespace boost::spirit::classic;
  using boost::spirit::classic::strlit;
  auto one_or_more_alpha_numeric = +alpha_p >> *(alpha_p | digit_p | punct_p);
  auto alphanumeric_path = chlit<char>('/') >> *(one_or_more_alpha_numeric >> *(chlit<char>('/')));
  auto paths_validos = (strlit<const char*>("/__endpoints") | alphanumeric_path );
  return parse(path.c_str(), paths_validos
               ).full;
}

bool Validations::is_valid_email(const std::string &email) {
  using namespace boost::spirit::classic;
  auto at_least_one_alphanumeric = +(+alpha_p | +digit_p);
  auto parse_result = parse(email.c_str(),
                            at_least_one_alphanumeric >> '@' >> +(at_least_one_alphanumeric >> '.')
                                                      >> at_least_one_alphanumeric);
  return parse_result.full;
}
void Validations::sanitize_path(std::string &path) {
  bool endsWithSlash = boost::algorithm::ends_with(path, "/");
  if (endsWithSlash) {
    boost::algorithm::replace_last(path, "/", "");
  }
}
}