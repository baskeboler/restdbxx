//
// Created by victor on 11/7/17.
//

#include "BaseRequestHandler.h"

namespace restdbxx {

bool BaseRequestHandler::not_found() const {
  using namespace proxygen;
  auto db = DbManager::get_instance();
  return (this->_method == HTTPMethod::GET || this->_method == HTTPMethod::PUT
      || this->_method == HTTPMethod::DELETE) && !db->path_exists(this->_path);
}
void BaseRequestHandler::sendEmptyContentResponse(int status, const std::string &message) const {
  proxygen::ResponseBuilder(downstream_)
      .status(status, message)
      .sendWithEOM();

}
void BaseRequestHandler::sendJsonResponse(const folly::dynamic &json, int status, const std::string &message) const {
  auto jsonStr = toPrettyJson(json);
  proxygen::ResponseBuilder(downstream_)
      .status(status, message)
      .header(proxygen::HTTP_HEADER_CONTENT_TYPE, "application/json")
      .header(proxygen::HTTP_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN, "*")
      .body(jsonStr.c_str())
      .sendWithEOM();

}
void BaseRequestHandler::sendStringResponse(const std::string &body, int status, const std::string &message) const {
  proxygen::ResponseBuilder(downstream_)
      .status(status, message)
      .body("dont delete root")
      .sendWithEOM();
}
BaseRequestHandler::BaseRequestHandler(): RequestHandler(){

}
folly::dynamic BaseRequestHandler::parseBody() const {
  if (_body)
    return folly::parseJson(_body->moveToFbString().toStdString());
  return nullptr;
}
}