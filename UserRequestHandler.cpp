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

}
void UserRequestHandler::onBody(std::unique_ptr<folly::IOBuf> body) noexcept {
  if (_body) {
    _body->prependChain(std::move(body));
  } else {
    _body = std::move(body);
  }
}
void UserRequestHandler::onRequest(std::unique_ptr<proxygen::HTTPMessage> headers) noexcept {
  VLOG(google::GLOG_INFO) << "baskeboler@gmail.com" << Validations::is_valid_email("baskeboler@gmail.com");

  VLOG(google::GLOG_INFO) << "baskeboler@gmail.com*" << Validations::is_valid_email("baskeboler@gmail.com*");
  auto m = headers->getMethodString();
  auto path = headers->getPath();

  if (m == "GET") {
    std::vector<std::string> parts;
    boost::algorithm::split(parts, path, boost::is_any_of("/"), boost::algorithm::token_compress_on);
    auto i = parts.rbegin();
    if (i != parts.rend()) {
      std::string username = *i;
      auto db = DbManager::get_instance();
      auto result = db->get_user(username);
      if (result) {
        ResponseBuilder(downstream_)
            .status(200, "OK")
            .header(proxygen::HTTPHeaderCode::HTTP_HEADER_CONTENT_TYPE, "application/json")
            .body(folly::toPrettyJson(result.value()))
            .sendWithEOM();
      } else {
        notFound();
      }
    } else {
      notFound();
    }
  }
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