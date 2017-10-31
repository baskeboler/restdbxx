//
// Created by victor on 24/10/17.
//

#include "RestDbRequestHandler.h"
#include "DbManager.h"
#include <folly/json.h>
#include <folly/ExceptionWrapper.h>
#include <proxygen/httpserver/ResponseBuilder.h>
namespace restdbxx {
static const char *INDEX_BODY = "<html>"
    "<head>"
    "</head>"
    "<body>"
    "<h1>RESTDB INDEX</h1>"
    "</body>"
    "</html>";
void RestDbRequestHandler::onRequest(std::unique_ptr<proxygen::HTTPMessage> headers) noexcept {
  auto db = DbManager::get_instance();

  if (headers->getMethod()) {
    _method = headers->getMethod().get();
  }
  _path = headers->getPath();
  auto p = headers->getPath();
  if ((_method == proxygen::HTTPMethod::GET || _method == proxygen::HTTPMethod::PUT
      || _method == proxygen::HTTPMethod::DELETE) && !db->path_exists(p)) {
    proxygen::ResponseBuilder(downstream_)
        .status(404, "Not Found")
        .sendWithEOM();
    return;
  }
  if (_method == proxygen::HTTPMethod::GET) {
    if (p == "/") {
      proxygen::ResponseBuilder(downstream_)
          .status(200, "OK")
          .body(INDEX_BODY)
          .sendWithEOM();
      return;
    }

    folly::try_and_catch<std::exception, std::runtime_error>([&]() {
        auto json = *db->get_path(p);
        auto jsonStr = folly::toPrettyJson(json);
        proxygen::ResponseBuilder(downstream_)
            .status(200, "OK")
            .body(jsonStr.c_str())
            .sendWithEOM();
      })/*.handle([&](std::exception& e) {
        LOG(INFO) << e.what();

        proxygen::ResponseBuilder(downstream_)
            .status(500, "mayhem")
            .body("dont delete root")
            .sendWithEOM();
      })*/;

  } else if (_method == proxygen::HTTPMethod::DELETE) {
    if (p == "/") {
      proxygen::ResponseBuilder(downstream_)
          .status(500, "mayhem")
          .body("dont delete root")
          .sendWithEOM();
      return;
    }

    folly::try_and_catch<std::exception, std::runtime_error>([&]() {
         db->remove(p);
        //auto jsonStr = folly::toPrettyJson(json);
        proxygen::ResponseBuilder(downstream_)
            .status(200, "OK")
            .sendWithEOM();
      })/*.handle([&](std::exception& e) {
        LOG(INFO) << e.what();

        proxygen::ResponseBuilder(downstream_)
            .status(500, "mayhem")
            .body("mierda")
            .sendWithEOM();
      })*/;

  }

}
void RestDbRequestHandler::onBody(std::unique_ptr<folly::IOBuf> body) noexcept {
  if (_body)
    _body->prependChain(std::move(body));
  else
    _body = std::move(body);
}

void RestDbRequestHandler::onUpgrade(proxygen::UpgradeProtocol prot) noexcept {

}

void RestDbRequestHandler::onEOM() noexcept {
  if (_method == proxygen::HTTPMethod::POST) {

    folly::try_and_catch<std::exception, std::runtime_error>([&]() {

      auto str = _body->moveToFbString();
      auto obj = folly::parseJson(str);
      auto db = DbManager::get_instance();
      db->post(_path, obj);
      proxygen::ResponseBuilder(downstream_)
          .status(201, "Created")
          .header(proxygen::HTTPHeaderCode::HTTP_HEADER_CONTENT_TYPE, "application/json")
          .body(str)
          .sendWithEOM();
    });
  } else if (_method == proxygen::HTTPMethod::PUT) {
    folly::try_and_catch<std::exception, std::runtime_error>([&]() {

      auto str = _body->moveToFbString();
      auto obj = folly::parseJson(str);
      auto db = DbManager::get_instance();
      db->put(_path, obj);
      proxygen::ResponseBuilder(downstream_)
          .status(200, "OK")
          .header(proxygen::HTTPHeaderCode::HTTP_HEADER_CONTENT_TYPE, "application/json")
          .body(str)
          .sendWithEOM();
    });
  }
}

void RestDbRequestHandler::requestComplete()noexcept {
  delete this;
}

void RestDbRequestHandler::onError(proxygen::ProxygenError err) noexcept {
  delete this;
}
}