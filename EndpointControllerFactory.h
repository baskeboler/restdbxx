//
// Created by victor on 11/11/17.
//

#ifndef RESTDBXX_ENDPOINTCONTROLLERFACTORY_H
#define RESTDBXX_ENDPOINTCONTROLLERFACTORY_H
#include <proxygen/httpserver/RequestHandlerFactory.h>

namespace restdbxx {
class EndpointControllerFactory : public proxygen::RequestHandlerFactory {
 public:
  void onServerStart(folly::EventBase *evb)noexcept override;
  void onServerStop() noexcept override;
  proxygen::RequestHandler *onRequest(proxygen::RequestHandler *handler,
                                      proxygen::HTTPMessage *message) noexcept override;

};
}

#endif //RESTDBXX_ENDPOINTCONTROLLERFACTORY_H
