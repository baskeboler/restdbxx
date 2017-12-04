//
// Created by victor on 11/7/17.
//

#ifndef RESTDBXX_BASEREQUESTHANDLER_H
#define RESTDBXX_BASEREQUESTHANDLER_H

//#include <proxygen/lib/http/HTTPMessage.h>
#include <proxygen/lib/http/HTTPMethod.h>
#include <folly/dynamic.h>
#include <proxygen/httpserver/RequestHandler.h>
#include <proxygen/httpserver/ResponseBuilder.h>
#include "DbManager.h"
#include <folly/Try.h>

namespace restdbxx {
const std::string HTTP_MESSAGE_OK = "OK";

using proxygen::RequestHandler;

class not_found_exception : public std::runtime_error {
 public:
  not_found_exception() : std::runtime_error("not found") {}
  virtual ~not_found_exception() = default;
};
class BaseRequestHandler: public proxygen::RequestHandler {
 public:
  BaseRequestHandler();
  virtual ~BaseRequestHandler() = default;
  virtual void setResponseHandler(proxygen::ResponseHandler *handler)noexcept override {
    RequestHandler::setResponseHandler(handler);
  }
  virtual void onEgressPaused()noexcept override {
    RequestHandler::onEgressPaused();
  }
  virtual void onEgressResumed()noexcept override {
    RequestHandler::onEgressResumed();
  }
 protected:
  std::unique_ptr<folly::IOBuf> _body;
  std::unique_ptr<proxygen::HTTPMessage> _headers;
  proxygen::HTTPMethod _method;
  std::string _path;
  virtual bool not_found() const;
  folly::Try<folly::dynamic> parseBody();
   bool is_endpoint_add = false;

  virtual void sendEmptyContentResponse(int status, const std::string &message) const;

  virtual void sendJsonResponse(const folly::dynamic &json, int status=200, const std::string &message=HTTP_MESSAGE_OK) const;
  void sendNotFound() const {
    sendEmptyContentResponse(404, "Not Found");
  }
  virtual void sendStringResponse(const std::string &body, int status=200, const std::string &message=HTTP_MESSAGE_OK) const;
};

}

#endif //RESTDBXX_BASEREQUESTHANDLER_H
