//
// Created by victor on 24/10/17.
//

#include "RestDbRequestHandler.h"
#include "DbManager.h"
#include "Validations.h"
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
  Validations::sanitize_path(_path);
  //auto p = headers->getPath();
  if (headers->getHeaders().exists("RESTDBXX_ADD_ENDPOINT"))
    is_endpoint_add = true;

  switch (_method) {
    case HTTPMethod::GET:
      if (_path == "") {
        sendStringResponse(INDEX_BODY);
        return;
      }
      if (db->is_endpoint(_path)) {
        std::vector<folly::dynamic> all;
        db->get_all(_path, all);
        sendJsonResponse(folly::dynamic::array(all));
      }
      folly::try_and_catch<std::exception, std::runtime_error>([&]() {
        auto json = *db->get(_path);
        sendJsonResponse(json);
      });
      break;
    case HTTPMethod::POST:break;
    case HTTPMethod::PUT:break;
    case HTTPMethod::DELETE:
      if (_path == "") {
        sendStringResponse("dont delete root", 500, "not cool, bro");
        return;
      }

      folly::try_and_catch<std::exception, std::runtime_error>([&]() {
        db->remove(_path);

        sendEmptyContentResponse(200, "OK");
      });
      break;
    default: break;
  }

  if (not_found()) {
    sendEmptyContentResponse(404, "Not Found");

  }

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
      auto copy = _body->cloneCoalescedAsValue();
      auto obj = folly::parseJson(copy.moveToFbString().toStdString());
      auto db = DbManager::get_instance();
      db->post(_path, obj);
      sendJsonResponse(obj, 201, "Created");

    });
  } else if (_method == HTTPMethod::PUT) {
    folly::try_and_catch<std::exception, std::runtime_error>([&]() {

      std::string str;
      auto copy = _body->cloneCoalescedAsValue();
      auto obj = folly::parseJson(copy.moveToFbString().toStdString());
      auto db = DbManager::get_instance();
      db->put(_path, obj);
      sendJsonResponse(obj);

    });
  } else {
    // nothing
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
RestDbRequestHandler::RestDbRequestHandler(): BaseRequestHandler() {}
}