//
// Created by victor on 11/11/17.
//

#include <folly/futures/Promise.h>
#include <folly/io/async/EventBaseManager.h>
#include "EndpointController.h"
#include "Validations.h"

namespace restdbxx {

void EndpointController::onRequest(std::unique_ptr<proxygen::HTTPMessage> headers) noexcept {
  _method = headers->getMethod() ? headers->getMethod().get() : proxygen::HTTPMethod::GET;
  _path = headers->getPath();
  Validations::sanitize_path(_path);

  switch (_method) {
    case proxygen::HTTPMethod::GET:
      if (_path == ENDPOINTS_PATH()) {
        folly::Promise<folly::dynamic> p;
        auto future = p.getFuture();
        folly::EventBaseManager::get()->getEventBase()->runInLoop([p = std::move(p), this]() mutable {
          p.setValue(get_endpoints_dynamic());
        });
        future.then([this](folly::dynamic &result) {
          sendJsonResponse(result);

        });
        return;
      }
      break;
    default:sendEmptyContentResponse(500, "internal error");
  }

}
folly::dynamic EndpointController::get_endpoints_dynamic() const {
  auto db = DbManager::get_instance();
  std::vector<folly::dynamic> endpoint_descrs;
  db->get_all(ENDPOINTS_PATH(), endpoint_descrs);
  folly::dynamic result = folly::dynamic::array(endpoint_descrs);
  return result;
}
void EndpointController::onBody(std::unique_ptr<folly::IOBuf> body) noexcept {
  if (_body) {
    _body->prependChain(std::move(body));
  } else {
    _body = std::move(body);
  }
}
void EndpointController::onUpgrade(proxygen::UpgradeProtocol prot) noexcept {

}
void EndpointController::onEOM() noexcept {
  if (_method == proxygen::HTTPMethod::POST) {
    processPost();
    return;
  }
}
void EndpointController::requestComplete()noexcept {
  delete this;
}
void EndpointController::onError(proxygen::ProxygenError err) noexcept {
  sendEmptyContentResponse(500, "internal error");
}
const std::string &EndpointController::ENDPOINTS_PATH() {
  static const std::string ENDPOINTS_PATH = "/__endpoints";
  return ENDPOINTS_PATH;
}
void EndpointController::processPost() {
  folly::dynamic json = folly::dynamic::object();
  json = parseBody();
  if (json.isObject() && !json.empty() && json.at("url").isString()) {
    std::string url = json["url"].asString();
    auto db = DbManager::get_instance();
    db->add_endpoint(url);
    auto retJson = db->get_endpoint(url);
    sendJsonResponse(retJson, 201, "endpoint created");
    return;
  }
  sendEmptyContentResponse(500, "Bad request");
  return;

}
}