//
// Created by victor on 24/10/17.
//

#include "LoggingFilter.h"
#include <proxygen/lib/utils/Logging.h>
#include <glog/logging.h>

namespace restdbxx {
using proxygen::Filter;

LoggingFilter::LoggingFilter(proxygen::RequestHandler *upstream): Filter(upstream)  {

}
void LoggingFilter::onRequest(std::unique_ptr<proxygen::HTTPMessage> headers) noexcept {

  //proxygen::
  VLOG(google::GLOG_INFO) << "Handling request for " << headers->getMethodString()
            << " " << headers->getPath();
  headers->getHeaders().forEach([](auto &k, auto &v) {
    LOG(INFO) << k << ": " << v;
  });
  Filter::onRequest(std::move(headers));
}
void LoggingFilter::onBody(std::unique_ptr<folly::IOBuf> body) noexcept {

  //std::string buffer_copy;
  //body->copyBuffer();
  folly::IOBuf value = body->cloneCoalescedAsValue();
  LOG(INFO) << proxygen::IOBufPrinter::printChainInfo(&value);
  LOG(INFO) << proxygen::IOBufPrinter::printHexFolly(&value);
  Filter::onBody(std::move(body));

}
void LoggingFilter::sendHeaders(proxygen::HTTPMessage &msg) noexcept {
  msg.getHeaders().forEach([](auto &k, auto &v) {
    LOG(INFO) << k << ": " << v;
  });
  Filter::sendHeaders(msg);
}
void LoggingFilter::sendBody(std::unique_ptr<folly::IOBuf> body) noexcept {

  folly::IOBuf value = body->cloneCoalescedAsValue();

  LOG(INFO) << proxygen::IOBufPrinter::printHexFolly(&value);
  Filter::sendBody(std::move(body));

}
void LoggingFilter::onEgressPaused() noexcept {
  LOG(INFO) << "onEgressPaused";

  Filter::onEgressPaused();
}
void LoggingFilter::onEgressResumed() noexcept {
  LOG(INFO) << "onEgressResumed";
  Filter::onEgressResumed();
}
}