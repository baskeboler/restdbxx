//
// Created by victor on 11/13/17.
//

#include "FiltersFactory.h"
#include "LoggingFilter.h"
#include "CorsFilter.h"
#include "AuthenticationFilter.h"

namespace restdbxx {

void FiltersFactory::onServerStart(folly::EventBase *evb) noexcept {

}
void FiltersFactory::onServerStop()noexcept {

}
proxygen::RequestHandler *FiltersFactory::onRequest(proxygen::RequestHandler *handler,
                                                    proxygen::HTTPMessage *message)noexcept {
  return new LoggingFilter(new CorsFilter(new AuthenticationFilter(handler)));
}
}