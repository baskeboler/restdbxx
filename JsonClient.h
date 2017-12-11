//
// Created by victor on 11/25/17.
//

#ifndef RESTDBXX_JSONCLIENT_H
#define RESTDBXX_JSONCLIENT_H

#include <proxygen/lib/http/HTTPConnector.h>
#include <folly/dynamic.h>
#include <proxygen/lib/http/session/HTTPTransaction.h>
#include <folly/futures/SharedPromise.h>
#include <folly/json.h>
#include <proxygen/lib/http/session/HTTPUpstreamSession.h>
#include <folly/io/async/EventBaseManager.h>
#include <proxygen/lib/utils/URL.h>
#include <folly/io/async/SSLOptions.h>
#include <ctime>

namespace restdbxx {
class JsonClient : public proxygen::HTTPConnector::Callback {
 public:
  explicit JsonClient(folly::HHWheelTimer *timer);

  void fetch(proxygen::HTTPMessage *req, folly::Promise<folly::dynamic> &promise);
  double get_elapsed() {
    return (_req_end - _req_start) / (double) (CLOCKS_PER_SEC / 1000);
  }

  //HTTPConnecto methods
  void connectSuccess(proxygen::HTTPUpstreamSession *session)noexcept override;
  void connectError(const folly::AsyncSocketException &ex) noexcept override;
  void onResponse(folly::dynamic &obj);
  void onError(const std::exception &e);

  virtual ~JsonClient();
 private:
  folly::Promise<folly::dynamic> _promise;
  proxygen::HTTPConnector _connector;
  class CustomHandler : public proxygen::HTTPTransactionHandler {
   private:
    JsonClient &_parent;
//    folly::SharedPromise<folly::dynamic> promise;
    std::unique_ptr<folly::IOBuf> _response_body;
   public:
    CustomHandler(JsonClient &parent) : _parent{parent} {}
    // HTTPTransactionHandler methods
    void setTransaction(proxygen::HTTPTransaction *txn) noexcept override;
    void detachTransaction() noexcept override;
    void onHeadersComplete(std::unique_ptr<proxygen::HTTPMessage> msg) noexcept override;
    void onBody(std::unique_ptr<folly::IOBuf> chain)  noexcept override;
    void onTrailers(std::unique_ptr<proxygen::HTTPHeaders> trailers) noexcept override;
    void onEOM() noexcept override;
    void onUpgrade(proxygen::UpgradeProtocol protocol) noexcept override;
    void onError(const proxygen::HTTPException &error) noexcept override;
    void onEgressPaused() noexcept override;

    void onEgressResumed() noexcept override;
  };

  class SessionWrapper : public proxygen::HTTPSession::InfoCallback {
   private:
    proxygen::HTTPUpstreamSession *session_{nullptr};

   public:
    explicit SessionWrapper(proxygen::HTTPUpstreamSession *session)
        : session_(session) {
      session_->setInfoCallback(this);
    }

    ~SessionWrapper() override {
      if (session_) {
        session_->drain();
      }
    }

    proxygen::HTTPUpstreamSession *operator->() const {
      return session_;
    }

    // Note: you must not start any asynchronous work from onDestroy()
    void onDestroy(const proxygen::HTTPSessionBase &) override {
      session_ = nullptr;
    }
  };

  void start_timer() {
    _req_start = std::clock();
  }
  void finish_timer() {
    _req_end = std::clock();
  }
  std::unique_ptr<proxygen::HTTPMessage> _request;
  proxygen::HTTPTransaction *_trx;
  CustomHandler trx_handler;
  std::clock_t _req_end;
  std::clock_t _req_start;
};

}
#endif //RESTDBXX_JSONCLIENT_H
