//
// Created by victor on 11/25/17.
//

#ifndef RESTDBXX_XKCDREQUESTHANDLER_H
#define RESTDBXX_XKCDREQUESTHANDLER_H
#include <proxygen/httpserver/RequestHandlerFactory.h>
#include "BaseRequestHandler.h"
#include "JsonClient.h"
namespace restdbxx {

class XkcdRequestHandler : public BaseRequestHandler {
 public:
  XkcdRequestHandler(const std::string &basic_string, folly::HHWheelTimer *pTimer);
  ~XkcdRequestHandler() override = default;
  void onRequest(std::unique_ptr<proxygen::HTTPMessage> headers) noexcept override;
  void onBody(std::unique_ptr<folly::IOBuf> body) noexcept override;
  void onUpgrade(proxygen::UpgradeProtocol prot) noexcept override;
  void onEOM() noexcept override;
  void requestComplete()noexcept override;
  void onError(proxygen::ProxygenError err) noexcept override;

 private:
  std::shared_ptr<JsonClient> client;
  std::string _mount_path;
};

class XkcdRequestHandlerFactory : public proxygen::RequestHandlerFactory {
 public:
  XkcdRequestHandlerFactory(const std::string &_mount_path);
  void onServerStart(folly::EventBase *evb) noexcept override;
  void onServerStop() noexcept override;
  RequestHandler *onRequest(RequestHandler *handler, proxygen::HTTPMessage *message)noexcept override;
  std::string _mount_path;
  struct TimerWrapper {
    folly::HHWheelTimer::UniquePtr _timer;
  };
  folly::ThreadLocal<TimerWrapper> timer;
};
}

#endif //RESTDBXX_XKCDREQUESTHANDLER_H
