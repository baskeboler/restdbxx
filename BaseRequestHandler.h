//
// Created by victor on 11/7/17.
//

#ifndef RESTDBXX_BASEREQUESTHANDLER_H
#define RESTDBXX_BASEREQUESTHANDLER_H

#include <proxygen/lib/http/HTTPMessage.h>
#include <proxygen/lib/http/HTTPMethod.h>
#include <folly/dynamic.h>
#include <proxygen/httpserver/RequestHandler.h>
#include <proxygen/httpserver/ResponseBuilder.h>
#include "DbManager.h"

namespace restdbxx {
const std::string HTTP_MESSAGE_OK = "OK";

using proxygen::RequestHandler;
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
  folly::dynamic parseBody() const {
    if (_body)
      return folly::parseJson(_body->moveToFbString().toStdString());
    return nullptr;
  }
   bool is_endpoint_add = false;

  virtual void sendEmptyContentResponse(int status, const std::string &message) const;

  virtual void sendJsonResponse(const folly::dynamic &json, int status=200, const std::string &message=HTTP_MESSAGE_OK) const;

  virtual void sendStringResponse(const std::string &body, int status=200, const std::string &message=HTTP_MESSAGE_OK) const;
};

}

#endif //RESTDBXX_BASEREQUESTHANDLER_H
