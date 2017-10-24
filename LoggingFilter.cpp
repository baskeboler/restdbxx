//
// Created by victor on 24/10/17.
//

#include "LoggingFilter.h"
#include <proxygen/lib/utils/Logging.h>
namespace restdbxx{
using proxygen::Filter;

LoggingFilter::LoggingFilter(proxygen::RequestHandler *upstream) : Filter(upstream) {}
void LoggingFilter::onRequest(std::unique_ptr<proxygen::HTTPMessage> headers) noexcept {

  //proxygen::
  LOG(INFO) << "Handling request for " << headers->getMethodString()
            << " " << headers->getPath();
  headers->getHeaders().forEach([](auto &k, auto& v){
    LOG(INFO) << k << ": " << v;
  });
  Filter::onRequest(std::move(headers));
}
void LoggingFilter::onBody(std::unique_ptr<folly::IOBuf> body) noexcept {

  LOG(INFO) << proxygen::IOBufPrinter::printChainInfo(body.get());
  LOG(INFO) << proxygen::IOBufPrinter::printHexFolly(body.get());
  Filter::onBody(std::move(body));

}
void LoggingFilter::sendHeaders(proxygen::HTTPMessage &msg) noexcept {
  msg.getHeaders().forEach([](auto &k, auto& v){
    LOG(INFO) << k << ": " << v;
  });
  Filter::sendHeaders(msg);
}
void LoggingFilter::sendBody(std::unique_ptr<folly::IOBuf> body) noexcept {
  LOG(INFO) << proxygen::IOBufPrinter::printHexFolly(body.get());
  Filter::sendBody(std::move(body));

}
}