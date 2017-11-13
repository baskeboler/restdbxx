//
// Created by victor on 11/11/17.
//

#include <boost/algorithm/string.hpp>
#include "EndpointControllerFactory.h"
#include "EndpointController.h"

namespace restdbxx {
void EndpointControllerFactory::onServerStart(folly::EventBase *evb) noexcept {

}
void EndpointControllerFactory::onServerStop() noexcept {

}
proxygen::RequestHandler *EndpointControllerFactory::onRequest(proxygen::RequestHandler *handler,
                                                               proxygen::HTTPMessage *message) noexcept {
  std::string path = message->getPath();
  bool match = boost::starts_with(path, EndpointController::ENDPOINTS_PATH());
  if (match) {
    VLOG(google::GLOG_INFO) << "path match, returning an EndpointController handler";
    return new EndpointController();
  }
  VLOG(google::GLOG_INFO) << "path does not match";

  return handler;
}
}
