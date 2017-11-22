//
// Created by victor on 11/13/17.
//

#include <proxygen/httpserver/filters/DirectResponseHandler.h>
#include "FiltersFactory.h"
#include "LoggingFilter.h"
#include "CorsFilter.h"
#include "AuthenticationFilter.h"

namespace restdbxx {
std::vector<std::string> FiltersFactory::_blacklist = {
    "/favicon.ico"
};
void FiltersFactory::onServerStart(folly::EventBase *evb) noexcept {

}
void FiltersFactory::onServerStop()noexcept {

}
proxygen::RequestHandler *FiltersFactory::onRequest(proxygen::RequestHandler *handler,
                                                    proxygen::HTTPMessage *message)noexcept {
  if (std::find(_blacklist.begin(), _blacklist.end(), message->getPath()) != _blacklist.end())
    return new proxygen::DirectResponseHandler(404, "Not Found", "Not Found");
  auto f = new LoggingFilter(new CorsFilter(new AuthenticationFilter(handler)));
  return f;
}
}