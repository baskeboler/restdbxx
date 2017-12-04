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
      } else {
        auto db = DbManager::get_instance();
        std::string pathTmp = _path;
        boost::erase_first(pathTmp, ENDPOINTS_PATH());
        if (db->is_endpoint(pathTmp)) {
          auto e = db->get_endpoint(pathTmp);
          sendJsonResponse(e);
          return;
        }
        sendEmptyContentResponse(404, "not found");
        return;
      }
      break;
    case proxygen::HTTPMethod::OPTIONS:
      proxygen::ResponseBuilder(downstream_)
          .status(200, "OK")
          .header(proxygen::HTTPHeaderCode::HTTP_HEADER_ACCESS_CONTROL_ALLOW_METHODS, "GET,POST,DELETE,PUT")
          .header(proxygen::HTTPHeaderCode::HTTP_HEADER_ACCESS_CONTROL_ALLOW_HEADERS, "content-type")
          .sendWithEOM();
      return;
    case proxygen::HTTPMethod::DELETE: {
      folly::Promise<folly::Unit> p;
      auto future = p.getFuture();
      folly::EventBaseManager::get()->getEventBase()->runInLoop([p = std::move(p), this]() mutable {
        auto db = DbManager::get_instance();
        try {
          std::string pathTmp(_path);
          boost::erase_first(pathTmp, ENDPOINTS_PATH());

          db->delete_endpoint(pathTmp);
          p.setValue();
        } catch (DbManagerException &e) {
          p.setException(e);
        }
      });
      future.then([this]() {
        sendEmptyContentResponse(200, "OK");
      }).onError([this](DbManagerException &e) {
        sendStringResponse(e.what(), 500, "error");
      });
      break;
    }
    default:break;
      //sendEmptyContentResponse(500, "internal error");
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
  delete this; //sendEmptyContentResponse(500, "internal error");
}
const std::string &EndpointController::ENDPOINTS_PATH() {
  static const std::string ENDPOINTS_PATH = "/__endpoints";
  return ENDPOINTS_PATH;
}
void EndpointController::processPost() {
  //folly::dynamic json = folly::dynamic::object();
  folly::Promise<folly::dynamic> promise;
  auto f = promise.getFuture();
  folly::EventBaseManager::get()->getEventBase()->runInLoop([p = std::move(promise), this]() mutable {
    p.setTry(parseBody());
  });
  f.then([this](folly::dynamic &json) {
    try {
      //auto &json = obj.value();
      if (json.isObject() && !json.empty() && json.at("url").isString()) {
        std::string url = json["url"].asString();
        auto db = DbManager::get_instance();
        db->add_endpoint(url);
        folly::dynamic retJson = db->get_endpoint(url);
//        p.setValue(retJson);
        return folly::makeFuture(retJson);
      } else
        return folly::makeFuture<folly::dynamic>(std::invalid_argument("Invalid json body"));
    } catch (const std::runtime_error &e) {
      //p.setException(e);
      return folly::makeFuture<folly::dynamic>(e);
    }
  }).then([this](folly::dynamic &ret) {
    sendJsonResponse(ret, 201, "endpoint created");
  }).onError([this](const std::runtime_error &e) {
    sendStringResponse("parsing error", 500, "Bad request");
  }).onError([this](const std::exception &e) {
    sendStringResponse("Internal error", 500, "Bad request");
  });
}
}