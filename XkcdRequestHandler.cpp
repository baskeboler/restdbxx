//
// Created by victor on 11/25/17.
//

#include <folly/executors/GlobalExecutor.h>
#include "XkcdRequestHandler.h"
#include "Validations.h"
#include <string>
using proxygen::HTTPMessage;
using proxygen::HTTPMethod;
using proxygen::ResponseBuilder;
using folly::HHWheelTimer;

namespace restdbxx {

void XkcdRequestHandler::onRequest(std::unique_ptr<proxygen::HTTPMessage> headers) noexcept {
  if (headers->getMethod() != HTTPMethod::GET && headers->getMethod() != HTTPMethod::OPTIONS) {
    ResponseBuilder(downstream_)
        .status(401, "Method not allowed")
        .sendWithEOM();
    return;
  }
  folly::Optional<int> comic = folly::none;
  if (headers->hasQueryParam("comic")) {

    int comicNumber = headers->getIntQueryParam("comic", -1);
    if (comicNumber != -1) comic = comicNumber;
  }
  folly::Promise<folly::dynamic> promise;
  auto f = promise.getFuture();
  folly::EventBaseManager::get()->getEventBase()
      ->runInLoop([comic, p = std::move(promise), this]() mutable {
        auto req = new HTTPMessage();
        std::stringstream ss;
        ss << "https://xkcd.com/";
        if (comic) {
          ss << comic.value() << "/";
        }
        ss << "info.0.json";
        req->setURL(ss.str());
        req->setMethod(HTTPMethod::GET);
        client->fetch(req, p);
      });
  f.then([this](folly::dynamic &comic) mutable {
    VLOG(google::GLOG_INFO) << "Got a comic!";
    ResponseBuilder(downstream_)
        .status(200, "OK")
        .header("RESTDBXX_QUERY_TIME", std::to_string(client->get_elapsed()))
        .header(proxygen::HTTPHeaderCode::HTTP_HEADER_CONTENT_TYPE, "application/json")
        .body(folly::toPrettyJson(comic))
        .sendWithEOM();
    return;
  }).onError([this](const std::runtime_error &e) {
    ResponseBuilder(downstream_)
        .status(500, "unexpected error")
        .body(e.what())
        .sendWithEOM();

  });

}
void XkcdRequestHandler::onBody(std::unique_ptr<folly::IOBuf> body) noexcept {
  if (_body) {
    _body->prependChain(std::move(body));
  } else {
    _body = std::move(body);
  }
}
void XkcdRequestHandler::onUpgrade(proxygen::UpgradeProtocol prot) noexcept {

}
void XkcdRequestHandler::onEOM() noexcept {

}
void XkcdRequestHandler::requestComplete()noexcept {
  delete this;
}
void XkcdRequestHandler::onError(proxygen::ProxygenError err) noexcept {
  delete this;
}

XkcdRequestHandler::XkcdRequestHandler(const std::string &path, folly::HHWheelTimer *pTimer) :
    _mount_path{path}, client{std::make_shared<JsonClient>(pTimer)} {

}

void XkcdRequestHandlerFactory::onServerStart(folly::EventBase *evb) noexcept {
  timer->_timer = HHWheelTimer::newTimer(
      evb,
      std::chrono::milliseconds(HHWheelTimer::DEFAULT_TICK_INTERVAL),
      folly::AsyncTimeout::InternalEnum::NORMAL,
      std::chrono::seconds(1));

}
void XkcdRequestHandlerFactory::onServerStop() noexcept {

}
RequestHandler *XkcdRequestHandlerFactory::onRequest(RequestHandler *handler, HTTPMessage *message) noexcept {
  std::string path = message->getPath();
  Validations::sanitize_path(path);
  if (boost::starts_with(path, _mount_path)) {
    return new XkcdRequestHandler(_mount_path, timer->_timer.get());;
  }
  return handler;
}
XkcdRequestHandlerFactory::XkcdRequestHandlerFactory(const std::string &_mount_path)
    : _mount_path(_mount_path) {}
}
