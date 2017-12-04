//
// Created by victor on 11/28/17.
//

#include "SimpleRoutingRequestHandlerFactory.h"
#include "Validations.h"
#include <folly/MapUtil.h>
namespace restdbxx {

void SimpleRoutingRequestHandlerFactory::onServerStart(folly::EventBase *evb) noexcept {

}
void SimpleRoutingRequestHandlerFactory::onServerStop()noexcept {

}
proxygen::RequestHandler *SimpleRoutingRequestHandlerFactory::onRequest(proxygen::RequestHandler *handler,
                                                                        proxygen::HTTPMessage *message) noexcept {
  std::string path = message->getPath();
  Validations::sanitize_path(path);
  auto f = [handler]() {
    return handler;
  };
  return folly::get_optional(_routing, path).value_or(f)();
}
}