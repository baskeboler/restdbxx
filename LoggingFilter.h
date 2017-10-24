//
// Created by victor on 24/10/17.
//

#ifndef RESTDBXX_LOGGINGFILTER_H
#define RESTDBXX_LOGGINGFILTER_H
#include <proxygen/httpserver/Filters.h>

namespace restdbxx {
class LoggingFilter : public proxygen::Filter{
 public:
  LoggingFilter(proxygen::RequestHandler *upstream);
  void onRequest(std::unique_ptr<proxygen::HTTPMessage> headers) noexcept override;
  void onBody(std::unique_ptr<folly::IOBuf> body) noexcept override;
  void sendHeaders(proxygen::HTTPMessage &msg) noexcept override;
  void sendBody(std::unique_ptr<folly::IOBuf> body) noexcept override;

};


}

#endif //RESTDBXX_LOGGINGFILTER_H
