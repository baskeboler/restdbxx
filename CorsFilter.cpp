//
// Created by victor on 11/7/17.
//

#include <proxygen/httpserver/Filters.h>
#include "CorsFilter.h"
namespace restdbxx {
CorsFilter::CorsFilter(proxygen::RequestHandler *upstream) : proxygen::Filter(upstream), _origin("*") {}

void CorsFilter::onRequest(std::unique_ptr<proxygen::HTTPMessage> headers) noexcept {
  bool exists = headers->getHeaders().exists(proxygen::HTTPHeaderCode::HTTP_HEADER_ORIGIN);
  if (exists) {
    _origin = headers->getHeaders().getSingleOrEmpty(proxygen::HTTPHeaderCode::HTTP_HEADER_ORIGIN);
  }
  upstream_->onRequest(std::move(headers));
}
void CorsFilter::sendHeaders(proxygen::HTTPMessage &msg) noexcept {
  msg.getHeaders().set(proxygen::HTTPHeaderCode::HTTP_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN, _origin);
  downstream_->sendHeaders(msg);
}
CorsFilter::~CorsFilter() = default;

}
