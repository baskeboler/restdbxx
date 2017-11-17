//
// Created by victor on 24/10/17.
//

#include "RestDbRequestHandler.h"
#include "DbManager.h"
#include "Validations.h"
#include <folly/json.h>
#include <folly/ExceptionWrapper.h>
#include <proxygen/httpserver/ResponseBuilder.h>
#include <folly/io/async/EventBaseManager.h>
#include <folly/futures/Promise.h>
namespace restdbxx {
using namespace proxygen;

static const char *INDEX_BODY = "<html>"
    "<head>"
    "</head>"
    "<body>"
    "<h1>RESTDB INDEX</h1>"
    "</body>"
    "</html>";

void RestDbRequestHandler::onRequest(std::unique_ptr<HTTPMessage> headers) noexcept {
  //_headers = std::move(headers);

  if (headers->getMethod()) {
    _method = headers->getMethod().get();
  }
  _path = headers->getPath();
  Validations::sanitize_path(_path);
  //auto p = headers->getPath();
  if (headers->getHeaders().exists("RESTDBXX_ADD_ENDPOINT"))
    is_endpoint_add = true;

}
folly::Future<std::vector<folly::dynamic>> RestDbRequestHandler::getListingFuture() const {
  folly::Promise<std::vector<folly::dynamic>> promise;
  auto future = promise.getFuture();
  folly::EventBaseManager::get()->getEventBase()->runInLoop([p = move(promise), this]() mutable {
    auto db = DbManager::get_instance();
    std::vector<folly::dynamic> all;
    db->get_all(_path, all);
    p.setValue(all);
  });
  return future;
}

void RestDbRequestHandler::onBody(std::unique_ptr<folly::IOBuf> body) noexcept {
  if (_body)
    _body->prependChain(std::move(body));
  else
    _body = std::move(body);
}

void RestDbRequestHandler::onEOM() noexcept {
  auto db = DbManager::get_instance();

  switch (_method) {
    case HTTPMethod::GET: {
      folly::dynamic json;
      if (_path == "") {
        sendStringResponse(INDEX_BODY);
        return;
      }
      if (db->is_endpoint(_path)) {
        getListingFuture().then([this](std::vector<folly::dynamic> &all) {
          sendJsonResponse(folly::dynamic::array(all));
        });
        return;
      }
      json = db->get(_path).value();
      sendJsonResponse(json);
      return;
    }
    case HTTPMethod::POST: {
      if (!_body) {
        sendStringResponse("Cannot post empty body", 500, "Error");
        return;
      }

      folly::Promise<folly::dynamic> promise;
      auto f = promise.getFuture();
      folly::EventBaseManager::get()->getEventBase()->runInLoop([p = std::move(promise), this]() mutable {
        folly::dynamic obj = folly::dynamic::object();
        obj = parseBody();

        auto db = DbManager::get_instance();
        db->post(_path, obj);
        p.setValue(obj);
      });
      f.then([this](folly::dynamic &obj) {

        sendJsonResponse(obj, 201, "Created");
      });
      return;
    }
    case HTTPMethod::PUT: {
      folly::Promise<folly::dynamic> promise;
      auto f = promise.getFuture();
      folly::EventBaseManager::get()->getEventBase()->runInLoop([p = std::move(promise), this]() mutable {

        folly::dynamic obj = folly::dynamic::object();
        obj = parseBody();
        auto db = DbManager::get_instance();
        db->put(_path, obj);
        p.setValue(obj);
      });

      f.then([this](folly::dynamic &obj) {

        sendJsonResponse(obj);
      });
      return;
      break;
    }
    case HTTPMethod::DELETE: {
      if (_path == "") {
        sendStringResponse("dont delete root", 500, "not cool, bro");
        return;
      }

      db->remove(_path);

      sendEmptyContentResponse(200, "OK");
      return;
    }
    case HTTPMethod::OPTIONS:
    case HTTPMethod::HEAD:
    case HTTPMethod::CONNECT:
    case HTTPMethod::TRACE:
    case HTTPMethod::PATCH: {   //default:
      sendEmptyContentResponse(500, "Unhandled request");
      return;
    }
  }

}

void RestDbRequestHandler::requestComplete() noexcept {
  delete this;
}

void RestDbRequestHandler::onError(ProxygenError err) noexcept {
  delete this;
}
void RestDbRequestHandler::onUpgrade(proxygen::UpgradeProtocol prot)noexcept {

}
RestDbRequestHandler::RestDbRequestHandler() : BaseRequestHandler() {}

}