//
// Created by victor on 24/10/17.
//

#ifndef RESTDBXX_RESTDBREQUESTHANDLER_H
#define RESTDBXX_RESTDBREQUESTHANDLER_H
#include <proxygen/httpserver/RequestHandler.h>

namespace restdbxx {

class RestDbRequestHandler: public proxygen::RequestHandler {
 public:
  void onRequest(std::unique_ptr<proxygen::HTTPMessage> headers) noexcept override;
  void onBody(std::unique_ptr<folly::IOBuf> body) noexcept override;
  void onUpgrade(proxygen::UpgradeProtocol prot) noexcept override;
  void onEOM() noexcept override;
  void requestComplete()noexcept  override;
  void onError(proxygen::ProxygenError err) noexcept override;


 private:
  std::unique_ptr<folly::IOBuf> _body;
  std::unique_ptr<proxygen::HTTPMessage> _headers;
  proxygen::HTTPMethod _method;
  std::string _path;
  bool not_found() const;
  bool is_endpoint_add = false;
};

}

#endif //RESTDBXX_RESTDBREQUESTHANDLER_H
