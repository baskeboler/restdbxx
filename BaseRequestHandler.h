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

class BaseRequestHandler: public proxygen::RequestHandler {
 public:
  virtual ~BaseRequestHandler() = default;
 protected:
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

#endif //RESTDBXX_BASEREQUESTHANDLER_H
