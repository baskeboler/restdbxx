//
// Created by victor on 11/15/17.
//

#ifndef RESTDBXX_AUTHENTICATIONFILTER_H
#define RESTDBXX_AUTHENTICATIONFILTER_H

#include <proxygen/httpserver/Filters.h>
namespace restdbxx {

class AuthenticationFilter : public proxygen::Filter {
 public:
  explicit AuthenticationFilter(proxygen::RequestHandler *upstream);
  void onRequest(std::unique_ptr<proxygen::HTTPMessage> headers) noexcept override;

  bool auth_required(std::unique_ptr<proxygen::HTTPMessage> headers) const {
    return false;
  }
  bool admin_required(std::unique_ptr<proxygen::HTTPMessage> headers) const {
    return false;
  }
  void onBody(std::unique_ptr<folly::IOBuf> body) noexcept override;

  void onUpgrade(proxygen::UpgradeProtocol protocol) noexcept override;
  void requestComplete() noexcept override;
  void onError(proxygen::ProxygenError err) noexcept override;
  void onEgressPaused() noexcept override;
  void onEgressResumed() noexcept override;

  void onEOM() noexcept override;
  static const std::string &RESTDBXX_TOKEN_HEADER_NAME();
  static const std::string &RESTDBXX_USERNAME_HEADER_NAME();
};

}

#endif //RESTDBXX_AUTHENTICATIONFILTER_H
