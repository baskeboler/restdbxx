//
// Created by victor on 11/15/17.
//

#include "AuthenticationRequestHandler.h"
#include "UserManager.h"

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

void AuthenticationRequestHandler::onBody(std::unique_ptr<folly::IOBuf> body) noexcept {
  if (_body) {
    _body->prependChain(std::move(body));
  } else {
    _body = std::move(body);
  }
}
void AuthenticationRequestHandler::onUpgrade(proxygen::UpgradeProtocol prot) noexcept {

}
void AuthenticationRequestHandler::onEOM() noexcept {

  if (_body && !_body->empty()) {
    folly::dynamic obj = folly::dynamic::object();
    obj = parseBody();
    if (!obj.isObject() || obj.empty()
        || !obj.at("username").isString()
        || !obj.at("password").isString()) {
      sendStringResponse("Bad request object", 401, "Unauthorized");
      return;
    }
    auto userCtrl = UserManager::get_instance();
    std::string username = obj.at("username").asString();
    std::string password = obj.at("password").asString();
    if (userCtrl->authenticate(username, password)) {
      auto token = userCtrl->get_access_token(username);
      sendJsonResponse(token->toDynamic());
      return;
    }
    sendStringResponse("Authentication failed", 401, "Unauthorized");
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