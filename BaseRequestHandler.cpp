//
// Created by victor on 11/7/17.
//

#include <folly/FBString.h>
#include "BaseRequestHandler.h"
#include <folly/Try.h>
#include <folly/Conv.h>
#include <folly/Range.h>
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
      .body(body)
      .sendWithEOM();
}
BaseRequestHandler::BaseRequestHandler() : RequestHandler(), _body(folly::IOBuf::create(1024)) {

}
folly::Try<folly::dynamic> BaseRequestHandler::parseBody() {
//  if (_body) {
//  std::string body_str = val->moveToFbString().toStdString();//->cloneCoalescedAsValue().toStdString();
  auto func = [b = _body->cloneCoalesced()]() {
    if (!b) {
      throw std::runtime_error("empty body");
    }
    std::string p(reinterpret_cast<const char *>(b->data()), b->length());
    //auto clone = _body->clone();
    return folly::parseJson(p);
  };
  return folly::makeTryWith(func);
//  }
//else return folly::Try::(std::runtime_error("empty body"));
}
}