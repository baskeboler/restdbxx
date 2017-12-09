//
// Created by victor on 11/15/17.
//

#include <folly/futures/Promise.h>
#include <folly/io/async/EventBaseManager.h>
#include "AuthenticationRequestHandler.h"
#include "UserManager.h"
#include <stdexcept>
namespace restdbxx {

void AuthenticationRequestHandler::onRequest(std::unique_ptr<proxygen::HTTPMessage> headers) noexcept {

  if (headers->getMethodString() == "OPTIONS") {
    proxygen::ResponseBuilder(downstream_)
        .status(200, "OK")
        .header(proxygen::HTTPHeaderCode::HTTP_HEADER_ALLOW, "POST, OPTIONS")
        .sendWithEOM();
    return;
  }
  if (headers->getPath() != "/authenticate" || headers->getMethodString() != "POST") {
    sendStringResponse("invalid request", 400, "Bad Request");
  }
}

void AuthenticationRequestHandler::onUpgrade(proxygen::UpgradeProtocol prot) noexcept {

}
void AuthenticationRequestHandler::onEOM() noexcept {

  if (_body && !_body->empty()) {
    folly::Promise<folly::dynamic> promise;
    auto f = promise.getFuture();
    folly::EventBaseManager::get()->getEventBase()->runInLoop([p = std::move(promise), this]() mutable {
      p.setTry(parseBody());
    });
    f.then([this](folly::dynamic &obj) {
      if (!obj.isObject() || obj.empty()
          || !obj.at("username").isString()
          || !obj.at("password").isString()) {
        throw std::invalid_argument("invalid json auth object");
      }
      auto userCtrl = UserManager::get_instance();
      std::string username = obj.at("username").asString();
      std::string password = obj.at("password").asString();
      if (userCtrl->authenticate(username, password)) {
        auto token = userCtrl->get_access_token(username);
        sendJsonResponse(token->toDynamic());

      } else {
        throw std::invalid_argument("Authentication failed");
      }
    }).onError([this](const std::exception &e) {
      sendStringResponse(e.what(), 401, "Unauthorized");
    });
    return;
  }
  sendEmptyContentResponse(400, "Bad Request");
}

void AuthenticationRequestHandler::requestComplete() noexcept {
  delete this;
}
void AuthenticationRequestHandler::onError(proxygen::ProxygenError err) noexcept {

  delete this;
}
AuthenticationRequestHandler::~AuthenticationRequestHandler() = default;

void AuthenticationRequestHandlerFactory::onServerStart(folly::EventBase *evb) noexcept {

}
void AuthenticationRequestHandlerFactory::onServerStop() noexcept {

}
RequestHandler *AuthenticationRequestHandlerFactory::onRequest(RequestHandler *handler,
                                                               proxygen::HTTPMessage *message) noexcept {
  if (message->getPath() == "/authenticate") {
    return new AuthenticationRequestHandler();
  }
  return handler;
}
}