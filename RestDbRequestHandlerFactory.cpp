//
// Created by victor on 24/10/17.
//

#include "RestDbRequestHandlerFactory.h"
#include "RestDbRequestHandler.h"
#include "LoggingFilter.h"
namespace restdbxx {
void RestDbRequestHandlerFactory::onServerStart(folly::EventBase *evb)noexcept {

}
void RestDbRequestHandlerFactory::onServerStop() noexcept {

}
proxygen::RequestHandler *RestDbRequestHandlerFactory::onRequest(proxygen::RequestHandler *handler,
                                                                 proxygen::HTTPMessage *message) noexcept {
  return new LoggingFilter(new RestDbRequestHandler());
}

}
