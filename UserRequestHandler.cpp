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

static const char *const USERS_PATH = "/__users";
void UserRequestHandler::onError(proxygen::ProxygenError err) noexcept {
  delete this;
}
void UserRequestHandler::requestComplete() noexcept {
  delete this;
}
void UserRequestHandler::onEOM() noexcept {
  if (_method == proxygen::HTTPMethod::POST && _path == USERS_PATH) {
    auto obj = parseBody();
    auto db = DbManager::get_instance();
    bool valid = validateUser(obj);
    auto maybeUser = valid ? db->get_user(obj["username"].asString()) : folly::none;
    if (maybeUser) {
      sendJsonResponse(folly::dynamic::object("error", true)
                           ("message", "user with same username already exists"),
                       403,
                       "Conflict");
    } else {
      auto user_manager = UserManager::get_instance();
      auto user = user_manager->create_user(obj.at("username").asString(), obj.at("password").asString());
      //db->post(_path, obj);
      sendJsonResponse(user->toDynamic(), 201, "Created");
    }
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
  _method = headers->getMethod().get();
  _path = headers->getPath();
  Validations::sanitize_path(_path);
  if (_method == proxygen::HTTPMethod::GET) {
    // we will respond to GET requests
    if (_path == USERS_PATH) {
      //return list
      std::vector<folly::dynamic> users;
      DbManager::get_instance()->get_all(USERS_PATH, users);
      folly::dynamic jsonResponse = folly::dynamic::array(users);
      sendJsonResponse(jsonResponse);
    } else {

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
          not_found();
        }
      } else {
        not_found();
      }
    }
  }
}

void UserRequestHandler::onUpgrade(proxygen::UpgradeProtocol prot) noexcept {
  // ...
}
UserRequestHandler::~UserRequestHandler() = default;
bool UserRequestHandler::validateUser(folly::dynamic &aDynamic) {
  return aDynamic.isObject()
      && !aDynamic.at("username").empty()
      && !aDynamic.at("password").empty();
}
UserRequestHandler::UserRequestHandler() = default;
}