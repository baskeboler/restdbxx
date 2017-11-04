//
// Created by victor on 4/11/17.
//

#ifndef RESTDBXX_USERREQUESTHANDLERFACTORY_H
#define RESTDBXX_USERREQUESTHANDLERFACTORY_H
#include <proxygen/httpserver/RequestHandlerFactory.h>

using proxygen::RequestHandlerFactory;

namespace restdbxx {

class UserRequestHandlerFactory : public RequestHandlerFactory{
 public:
  void onServerStart(folly::EventBase *evb) noexcept override;
  void onServerStop() noexcept override;
  proxygen::RequestHandler *onRequest(proxygen::RequestHandler *handler, proxygen::HTTPMessage *message) noexcept override;
  UserRequestHandlerFactory();
  virtual ~UserRequestHandlerFactory();
};

}
#endif //RESTDBXX_USERREQUESTHANDLERFACTORY_H
