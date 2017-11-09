//
// Created by victor on 24/10/17.
//

#ifndef RESTDBXX_RESTDBREQUESTHANDLER_H
#define RESTDBXX_RESTDBREQUESTHANDLER_H
#include <proxygen/httpserver/RequestHandler.h>
#include <folly/dynamic.h>
#include "BaseRequestHandler.h"

const std::string HTTP_MESSAGE_OK = "OK";
namespace restdbxx {

class RestDbRequestHandler: public BaseRequestHandler {
 public:
  RestDbRequestHandler();
  void onRequest(std::unique_ptr<proxygen::HTTPMessage> headers) noexcept override;
  void onBody(std::unique_ptr<folly::IOBuf> body) noexcept override;
  void onUpgrade(proxygen::UpgradeProtocol prot) noexcept override;
  void onEOM() noexcept override;
  void requestComplete()noexcept  override;
  void onError(proxygen::ProxygenError err) noexcept override;


 private:
  };

}

#endif //RESTDBXX_RESTDBREQUESTHANDLER_H
