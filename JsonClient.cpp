//
// Created by victor on 11/25/17.
//

#include "JsonClient.h"
#include <proxygen/lib/http/session/HTTPUpstreamSession.h>
namespace restdbxx {

void JsonClient::connectSuccess(proxygen::HTTPUpstreamSession *session) noexcept {
  _trx = session->newTransaction(&trx_handler);
  _trx->sendHeaders(*_request);
  auto method = _request->getMethodString();
  if (method == "POST" || method == "PUT") {
    // we nned to send body
    // not implemented yet.
  }
  _trx->sendEOM();
}
void JsonClient::connectError(const folly::AsyncSocketException &ex) noexcept {

}
JsonClient::JsonClient(folly::HHWheelTimer *timer) :
    _trx{nullptr},
    _connector{this, timer},
    trx_handler(*this) {

}
void JsonClient::fetch(proxygen::HTTPMessage *req, folly::Promise<folly::dynamic> &promise) {
  _request.reset(req);// = std::move(req);
  _promise = std::move(promise);
  auto evb = folly::EventBaseManager::get()->getEventBase();
  proxygen::URL url{_request->getURL()};
  _request->getHeaders().add(proxygen::HTTPHeaderCode::HTTP_HEADER_HOST, url.getHost());
  _request->getHeaders().add(proxygen::HTTPHeaderCode::HTTP_HEADER_CONTENT_TYPE, "application/json");

  folly::SocketAddress addr;
  addr.setFromHostPort(url.getHost(), url.getPort());
  const folly::AsyncSocket::OptionMap opts{
      {{SOL_SOCKET, SO_REUSEADDR}, 1}};

  if (url.isSecure()) {

    auto sslContext_ = std::make_shared<folly::SSLContext>();
    sslContext_->setOptions(SSL_OP_NO_COMPRESSION);
    sslContext_->setCipherList(folly::ssl::SSLCommonOptions::kCipherList);
    _connector.connectSSL(evb, addr, sslContext_, nullptr, std::chrono::milliseconds(1000), opts);
  } else {
    _connector.connect(evb, addr, std::chrono::milliseconds(1000), opts);
  }
  start_timer();

}
void JsonClient::onResponse(folly::dynamic &obj) {
  finish_timer();
  _promise.setValue(obj);
}
void JsonClient::onError(const std::exception &e) {
  finish_timer();
  _promise.setException(e);
}
JsonClient::~JsonClient() {
  VLOG(google::GLOG_INFO) << "destroying JsonClient";
//  if (_trx) delete _trx;
}

void JsonClient::CustomHandler::setTransaction(proxygen::HTTPTransaction *txn) noexcept {

}
void JsonClient::CustomHandler::detachTransaction() noexcept {

}
void JsonClient::CustomHandler::onHeadersComplete(std::unique_ptr<proxygen::HTTPMessage> msg) noexcept {
  msg->dumpMessage(0);
}

void JsonClient::CustomHandler::onBody(std::unique_ptr<folly::IOBuf> chain) noexcept {
  if (_response_body) {
    _response_body->prependChain(std::move(chain));
  } else {
    _response_body = std::move(chain);
  }
}
void JsonClient::CustomHandler::onTrailers(std::unique_ptr<proxygen::HTTPHeaders> trailers) noexcept {

}
void JsonClient::CustomHandler::onEOM()noexcept {
  auto f = [b = std::move(_response_body)]() {
    if (!b) throw std::runtime_error("body empty");
    std::string str(reinterpret_cast<const char * >
                    (b->data()), b->length());
    return folly::parseJson(str);
  };
  folly::Try<folly::dynamic> tryJson = folly::makeTryWith(f);
  if (tryJson.hasException()) {
    try {
      tryJson.throwIfFailed();
    } catch (std::exception &e) {

      _parent.onError(e);
    }
  } else if (tryJson.hasValue()) {
    _parent.onResponse(tryJson.value());
  } else {
    _parent.onError(std::runtime_error("failed to get response"));
  }
//  _req_end = std::clock();
}
void JsonClient::CustomHandler::onUpgrade(proxygen::UpgradeProtocol protocol) noexcept {

}
void JsonClient::CustomHandler::onError(const proxygen::HTTPException &error)noexcept {

}
void JsonClient::CustomHandler::onEgressPaused() noexcept {

}
void JsonClient::CustomHandler::onEgressResumed()noexcept {

}
}