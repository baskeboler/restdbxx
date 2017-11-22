//
// Created by victor on 11/13/17.
//

#ifndef RESTDBXX_FILTERSFACTORY_H
#define RESTDBXX_FILTERSFACTORY_H

#include <proxygen/httpserver/RequestHandlerFactory.h>
namespace restdbxx {
class FiltersFactory: public proxygen::RequestHandlerFactory {
 public:
  void onServerStart(folly::EventBase *evb) noexcept override;
  void onServerStop() noexcept override;
  proxygen::RequestHandler *onRequest(proxygen::RequestHandler *handler, proxygen::HTTPMessage *message) noexcept override;
 private:
  static std::vector<std::string> _blacklist;
};

}

#endif //RESTDBXX_FILTERSFACTORY_H
