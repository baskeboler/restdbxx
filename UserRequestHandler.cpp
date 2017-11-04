//
// Created by victor on 4/11/17.
//

#include "UserRequestHandler.h"
#include "UserManager.h"
#include "Validations.h"
#include <boost/algorithm/string.hpp>
#include <proxygen/httpserver/ResponseBuilder.h>
#include <folly/json.h>
using proxygen::ResponseBuilder;

namespace restdbxx {

void UserRequestHandler::onError(proxygen::ProxygenError err) noexcept {
  delete this;
}
void UserRequestHandler::requestComplete() noexcept {
  delete this;
}
void UserRequestHandler::onEOM() noexcept {
  if (_method == "POST" && _path == "/__users") {

    std::string str;
    auto copy = _body->cloneCoalescedAsValue();
    auto obj = folly::parseJson(copy.moveToFbString().toStdString());
    auto db = DbManager::get_instance();
    db->post(_path, obj);
    sendJsonResponse(obj, 201, "Created");
  }
}

void UserRequestHandler::onBody(std::unique_ptr<folly::IOBuf> body) noexcept {
  if (_body) {
    _body->prependChain(std::move(body));
  } else {
    _body = std::move(body);
  }
}
void UserRequestHandler::onRequest(std::unique_ptr<proxygen::HTTPMessage> headers) noexcept {
  _method = headers->getMethodString();
  _path = headers->getPath();
  Validations::sanitize_path(_path);
  if (_method == "GET") {
    std::vector<std::string> parts;
    boost::algorithm::split(parts, _path, boost::is_any_of("/"), boost::algorithm::token_compress_on);
    auto i = parts.rbegin();
    if (i != parts.rend()) {
      std::string username = *i;
      auto db = DbManager::get_instance();
      auto result = db->get_user(username);
      if (result) {
        sendJsonResponse(result.value());
      } else {
        notFound();
      }
    } else {
      notFound();
    }
  }
}
void UserRequestHandler::sendJsonResponse(folly::dynamic &result, int status, const std::string &message) const {
  ResponseBuilder(downstream_)
            .status(status, message)
            .header(proxygen::HTTP_HEADER_CONTENT_TYPE, "application/json")
            .body(folly::toPrettyJson(result))
            .sendWithEOM();
}

void UserRequestHandler::notFound() {
  ResponseBuilder(downstream_)
      .status(404, "not found")
      .sendWithEOM();
}
void UserRequestHandler::onUpgrade(proxygen::UpgradeProtocol prot) noexcept {
  // ...
}
UserRequestHandler::~UserRequestHandler() {

}
}