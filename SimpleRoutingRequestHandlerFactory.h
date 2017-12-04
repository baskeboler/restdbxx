//
// Created by victor on 11/28/17.
//

#ifndef RESTDBXX_SIMPLEROUTINGREQUESTHANDLERFACTORY_H
#define RESTDBXX_SIMPLEROUTINGREQUESTHANDLERFACTORY_H

#include <proxygen/httpserver/RequestHandlerFactory.h>
namespace restdbxx {
class SimpleRoutingRequestHandlerFactory : public proxygen::RequestHandlerFactory {
 public:
  typedef std::function<proxygen::RequestHandler *()> request_handler_object_factory;
  void onServerStart(folly::EventBase *evb) noexcept override;
  void onServerStop() noexcept override;

  proxygen::RequestHandler *onRequest(proxygen::RequestHandler *handler,
                                      proxygen::HTTPMessage *message) noexcept override;
  void registerHandler(const std::string &path, request_handler_object_factory &factory) {
    _routing.emplace(path, factory);
  }
 private:
  std::map<std::string, request_handler_object_factory> _routing;
};

};

#endif //RESTDBXX_SIMPLEROUTINGREQUESTHANDLERFACTORY_H
