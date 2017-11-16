//
// Created by victor on 11/15/17.
//

#ifndef RESTDBXX_AUTHENTICATIONREQUESTHANDLER_H
#define RESTDBXX_AUTHENTICATIONREQUESTHANDLER_H

#include <proxygen/httpserver/RequestHandlerFactory.h>
#include "BaseRequestHandler.h"
namespace restdbxx {
class AuthenticationRequestHandler : public BaseRequestHandler {
 public:
  void onRequest(std::unique_ptr<proxygen::HTTPMessage> headers) noexcept override;
  void onBody(std::unique_ptr<folly::IOBuf> body) noexcept override;
  void onUpgrade(proxygen::UpgradeProtocol prot) noexcept override;
  void onEOM() noexcept override;
  void requestComplete() noexcept override;
  void onError(proxygen::ProxygenError err) noexcept override;
  virtual ~AuthenticationRequestHandler();
};

class AuthenticationRequestHandlerFactory : public proxygen::RequestHandlerFactory {
 public:
  void onServerStart(folly::EventBase *evb) noexcept override;
  void onServerStop() noexcept override;
  RequestHandler *onRequest(RequestHandler *handler, proxygen::HTTPMessage *message) noexcept override;
};
}

#endif //RESTDBXX_AUTHENTICATIONREQUESTHANDLER_H
