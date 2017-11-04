//
// Created by victor on 24/10/17.
//

#ifndef RESTDBXX_RESTDBREQUESTHANDLER_H
#define RESTDBXX_RESTDBREQUESTHANDLER_H
#include <proxygen/httpserver/RequestHandler.h>
#include <folly/dynamic.h>

const std::string HTTP_MESSAGE_OK = "OK";
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
  void sendEmptyContentResponse(int status, const std::string &message) const;
  void sendJsonResponse(const folly::dynamic &json, int status=200, const std::string &message=HTTP_MESSAGE_OK) const;
  void sendStringResponse(const std::string &body, int status=200, const std::string &message=HTTP_MESSAGE_OK) const;
};

}

#endif //RESTDBXX_RESTDBREQUESTHANDLER_H
