//
// Created by victor on 24/10/17.
//

#ifndef RESTDBXX_RESTDBREQUESTHANDLERFACTORY_H
#define RESTDBXX_RESTDBREQUESTHANDLERFACTORY_H
#include <proxygen/httpserver/RequestHandlerFactory.h>

namespace restdbxx {
class RestDbRequestHandlerFactory: public proxygen::RequestHandlerFactory {
 public:
  void onServerStart(folly::EventBase *evb) noexcept override;
  void onServerStop() noexcept override;
  proxygen::RequestHandler *onRequest(proxygen::RequestHandler *handler, proxygen::HTTPMessage *message) noexcept override;

};

}

#endif //RESTDBXX_RESTDBREQUESTHANDLERFACTORY_H
