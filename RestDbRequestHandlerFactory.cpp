//
// Created by victor on 24/10/17.
//

#include "RestDbRequestHandlerFactory.h"
#include "RestDbRequestHandler.h"
#include "LoggingFilter.h"
#include "Validations.h"
#include <proxygen/httpserver/filters/DirectResponseHandler.h>
#include <boost/spirit/include/classic.hpp>

namespace restdbxx {
void RestDbRequestHandlerFactory::onServerStart(folly::EventBase *evb)noexcept {

}
void RestDbRequestHandlerFactory::onServerStop() noexcept {

}
proxygen::RequestHandler *RestDbRequestHandlerFactory::onRequest(proxygen::RequestHandler *handler,
                                                                 proxygen::HTTPMessage *message) noexcept {
  proxygen::RequestHandler* resultHandler = nullptr;
  std::string path = message->getPath();
  Validations::sanitize_path(path);
  if (Validations::is_valid_path(path)) {
    resultHandler = new RestDbRequestHandler();

  } else if (handler != nullptr) {
    resultHandler = handler;

  } else {
    resultHandler = new proxygen::DirectResponseHandler(500, "Wierd error", "not really");
  }
  return resultHandler;
}

}
