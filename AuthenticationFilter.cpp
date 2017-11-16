//
// Created by victor on 11/15/17.
//

#include <proxygen/httpserver/Filters.h>
#include <proxygen/httpserver/ResponseBuilder.h>
#include "AuthenticationFilter.h"
#include "UserManager.h"

namespace restdbxx {

void AuthenticationFilter::onRequest(std::unique_ptr<proxygen::HTTPMessage> headers) noexcept {
  using std::string;
  bool auth_headers_present = headers->getHeaders().exists(RESTDBXX_TOKEN_HEADER_NAME());
  auth_headers_present = auth_headers_present && headers->getHeaders().exists(RESTDBXX_USERNAME_HEADER_NAME());

  if (auth_headers_present) {
    string username = headers->getHeaders().getSingleOrEmpty(RESTDBXX_USERNAME_HEADER_NAME());
    string token = headers->getHeaders().getSingleOrEmpty(RESTDBXX_TOKEN_HEADER_NAME());

    auto user_manager = UserManager::get_instance();

    bool token_valid = user_manager->validate_access_token(username, token);
    if (!token_valid) {
      VLOG(google::GLOG_INFO) << "invalid token, rejecting request";
      upstream_->onError(proxygen::ProxygenError::kErrorMalformedInput);
      upstream_ = nullptr;

      proxygen::ResponseBuilder(downstream_)
          .status(401, "Authentication Failed")
          .body("invalid token")
          .sendWithEOM();
      return;
    } else {
      VLOG(google::GLOG_INFO) << "token is valid, proceed";
    }
  }
  proxygen::Filter::onRequest(std::move(headers));
}

AuthenticationFilter::AuthenticationFilter(proxygen::RequestHandler *upstream) : Filter(upstream) {}
const std::string &AuthenticationFilter::RESTDBXX_TOKEN_HEADER_NAME() {
  static const std::string value = "RESTDBXX_AUTH_TOKEN";
  return value;
}
const std::string &AuthenticationFilter::RESTDBXX_USERNAME_HEADER_NAME() {
  static const std::string value = "RESTDBXX_AUTH_USERNAME";
  return value;
}
void AuthenticationFilter::requestComplete() noexcept {
  downstream_ = nullptr;
  if (upstream_)
    upstream_->requestComplete();
  delete this;

}
void AuthenticationFilter::onError(proxygen::ProxygenError err) noexcept {
  downstream_ = nullptr;
  if (upstream_)
    upstream_->onError(err);
  delete this;

}
void AuthenticationFilter::onEgressPaused() noexcept {
  if (upstream_) {
    upstream_->onEgressPaused();
  }
}
void AuthenticationFilter::onEgressResumed() noexcept {
  if (upstream_) {
    upstream_->onEgressResumed();
  }
}
}