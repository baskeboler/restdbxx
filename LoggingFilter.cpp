//
// Created by victor on 24/10/17.
//

#include "LoggingFilter.h"
#include <proxygen/lib/utils/Logging.h>
#include <glog/logging.h>
#include <proxygen/httpserver/Filters.h>
#include <proxygen/httpserver/RequestHandler.h>
#include <proxygen/lib/http/HTTPMessage.h>

namespace restdbxx {
using proxygen::Filter;

LoggingFilter::LoggingFilter(proxygen::RequestHandler *upstream): Filter(upstream)  {

}
void LoggingFilter::onRequest(std::unique_ptr<proxygen::HTTPMessage> headers) noexcept {

  //proxygen::
  VLOG(google::GLOG_INFO) << "Handling request for " << headers->getMethodString()
            << " " << headers->getPath();
  headers->getHeaders().forEach([](auto &k, auto &v) {
    VLOG(google::GLOG_INFO) << k << ": " << v;
  });
  upstream_->onRequest(std::move(headers));
}
void LoggingFilter::onBody(std::unique_ptr<folly::IOBuf> body) noexcept {

  //std::string buffer_copy;
  //body->copyBuffer();
  folly::IOBuf value = body->cloneCoalescedAsValue();
  VLOG(google::GLOG_INFO) << proxygen::IOBufPrinter::printChainInfo(&value);
  VLOG(google::GLOG_INFO) << proxygen::IOBufPrinter::printHexFolly(&value);
  upstream_->onBody(std::move(body));

}
void LoggingFilter::sendHeaders(proxygen::HTTPMessage &msg) noexcept {
//  msg.getHeaders().forEach([](auto &k, auto &v) {
//    VLOG(google::GLOG_INFO) << k << ": " << v;
//  });
  msg.atomicDumpMessage(google::GLOG_INFO);
  downstream_->sendHeaders(msg);
}
void LoggingFilter::sendBody(std::unique_ptr<folly::IOBuf> body) noexcept {

  folly::IOBuf value = body->cloneCoalescedAsValue();

  VLOG(google::GLOG_INFO) << proxygen::IOBufPrinter::printHexFolly(&value);
  Filter::sendBody(std::move(body));

}
void LoggingFilter::onEgressPaused() noexcept {
  VLOG(google::GLOG_INFO) << "onEgressPaused";

  upstream_->onEgressPaused();
}
void LoggingFilter::onEgressResumed() noexcept {
  VLOG(google::GLOG_INFO) << "onEgressResumed";
  upstream_->onEgressResumed();
}
LoggingFilter::~LoggingFilter() = default;
}