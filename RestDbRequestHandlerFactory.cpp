//
// Created by victor on 24/10/17.
//

#include "RestDbRequestHandlerFactory.h"
#include "RestDbRequestHandler.h"
#include "LoggingFilter.h"
#include "Validations.h"
#include "CorsFilter.h"
#include <proxygen/httpserver/filters/DirectResponseHandler.h>
#include <boost/spirit/include/classic.hpp>
#include <proxygen/httpserver/Filters.h>

namespace restdbxx {
void RestDbRequestHandlerFactory::onServerStart(folly::EventBase *evb)noexcept {

  VLOG(google::GLOG_INFO) << "starting server ";

}
void RestDbRequestHandlerFactory::onServerStop() noexcept {

  VLOG(google::GLOG_INFO) << "stopping server ";
}
proxygen::RequestHandler *RestDbRequestHandlerFactory::onRequest(proxygen::RequestHandler *handler,
                                                                 proxygen::HTTPMessage *message) noexcept {
  proxygen::RequestHandler *resultHandler = nullptr;
  std::string path = message->getPath();
  Validations::sanitize_path(path);
  auto withFilter = [](auto handler) {
    return new LoggingFilter(new CorsFilter(handler));
  };
  if (Validations::is_valid_path(path)) {

    VLOG(google::GLOG_INFO) << "el path es valido ";
    resultHandler = withFilter(new RestDbRequestHandler());

  } else if (handler != nullptr) {
    VLOG(google::GLOG_WARNING) << "el path no es valido, pasando el control al siguiente handler";

    resultHandler = withFilter(handler);

  } else {
    VLOG(google::GLOG_WARNING) << "el path no es valido ";

    resultHandler = new proxygen::DirectResponseHandler(500, "Wierd error", "not really");
  }
  return resultHandler;
}

}
