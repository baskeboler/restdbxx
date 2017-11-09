//
// Created by victor on 11/7/17.
//

#ifndef RESTDBXX_CORSFILTER_H
#define RESTDBXX_CORSFILTER_H

#include <proxygen/httpserver/Filters.h>

namespace restdbxx {
class CorsFilter : public proxygen::Filter{
 public:
  ~CorsFilter() override;
  CorsFilter(proxygen::RequestHandler *upstream);
  void onRequest(std::unique_ptr<proxygen::HTTPMessage> headers) noexcept override;
  void sendHeaders(proxygen::HTTPMessage &msg) noexcept override;
 private:
  std::string _origin;
};

}

#endif //RESTDBXX_CORSFILTER_H
