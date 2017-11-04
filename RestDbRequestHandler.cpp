//
// Created by victor on 24/10/17.
//

#include "RestDbRequestHandler.h"
#include "DbManager.h"
#include <folly/json.h>
#include <folly/ExceptionWrapper.h>
#include <proxygen/httpserver/ResponseBuilder.h>
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
  auto db = DbManager::get_instance();
  //_headers = std::move(headers);

  if (headers->getMethod()) {
    _method = headers->getMethod().get();
  }
  _path = headers->getPath();
  //auto p = headers->getPath();
  if (headers->getHeaders().exists("RESTDBXX_ADD_ENDPOINT"))
    is_endpoint_add = true;

  switch (_method) {
    case HTTPMethod::GET:
      if (_path == "/") {
        ResponseBuilder(downstream_)
            .status(200, "OK")
            .body(INDEX_BODY)
            .sendWithEOM();
        return;
      }
      break;
    case HTTPMethod::POST:
      break;
    case HTTPMethod::PUT:
      break;
    case HTTPMethod::DELETE:
      break;
  }
  if (_method == HTTPMethod::GET) {


    folly::try_and_catch<std::exception, std::runtime_error>([&]() {
      auto json = *db->get(_path);
      auto jsonStr = folly::toPrettyJson(json);
      ResponseBuilder(downstream_)
          .status(200, "OK")
          .body(jsonStr.c_str())
          .sendWithEOM();
    })/*.handle([&](std::exception& e) {
        LOG(INFO) << e.what();

        ResponseBuilder(downstream_)
            .status(500, "mayhem")
            .body("dont delete root")
            .sendWithEOM();
      })*/;

  } else if (_method == HTTPMethod::POST) {

  } else if (_method == HTTPMethod::DELETE) {
    if (_path == "/") {
      ResponseBuilder(downstream_)
          .status(500, "mayhem")
          .body("dont delete root")
          .sendWithEOM();
      return;
    }

    folly::try_and_catch<std::exception, std::runtime_error>([&]() {
      db->remove(_path);
      //auto jsonStr = folly::toPrettyJson(json);
      ResponseBuilder(downstream_)
          .status(200, "OK")
          .sendWithEOM();
    })/*.handle([&](std::exception& e) {
        LOG(INFO) << e.what();

        ResponseBuilder(downstream_)
            .status(500, "mayhem")
            .body("mierda")
            .sendWithEOM();
      })*/;

  }

  if (not_found()) {
    ResponseBuilder(downstream_)
        .status(404, "Not Found")
        .sendWithEOM();
    return;
  }

}
bool RestDbRequestHandler::not_found() const {
  auto db = DbManager::get_instance();
  return (this->_method == HTTPMethod::GET || this->_method == HTTPMethod::PUT
      || this->_method == HTTPMethod::DELETE) && !db->path_exists(this->_path);
}

void RestDbRequestHandler::onBody(std::unique_ptr<folly::IOBuf> body) noexcept {
  if (_body)
    _body->prependChain(std::move(body));
  else
    _body = std::move(body);
}

void RestDbRequestHandler::onEOM() noexcept {

  if (_method == HTTPMethod::POST) {

    folly::try_and_catch<std::exception, std::runtime_error>([&]() {

      //folly::StringPiece s(_body->buffer(), _body->length());

      std::string str;
      this->_body->copyBuffer(str);
      auto obj = folly::parseJson(str);
      auto db = DbManager::get_instance();
      db->post(_path, obj);
      ResponseBuilder(downstream_)
          .status(201, "Created")
          .header(HTTPHeaderCode::HTTP_HEADER_CONTENT_TYPE, "application/json")
          .body(folly::toPrettyJson(obj))
          .sendWithEOM();
    });
  } else if (_method == HTTPMethod::PUT) {
    folly::try_and_catch<std::exception, std::runtime_error>([&]() {

      std::string str;
      this->_body->copyBuffer(str);
      auto obj = folly::parseJson(str);
      auto db = DbManager::get_instance();
      db->put(_path, obj);
      ResponseBuilder(downstream_)
          .status(200, "OK")
          .header(HTTPHeaderCode::HTTP_HEADER_CONTENT_TYPE, "application/json")
          .body(str)
          .sendWithEOM();
    });
  } else {
  }

}

void RestDbRequestHandler::requestComplete()noexcept {
  delete this;
}

void RestDbRequestHandler::onError(ProxygenError err) noexcept {
  delete this;
}
void RestDbRequestHandler::onUpgrade(proxygen::UpgradeProtocol prot)noexcept {

}
}