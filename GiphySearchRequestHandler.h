//
// Created by victor on 11/22/17.
//

#ifndef RESTDBXX_GIPHYSEARCHREQUESTHANDLER_H
#define RESTDBXX_GIPHYSEARCHREQUESTHANDLER_H
#include <proxygen/lib/http/session/HTTPSession.h>
#include <proxygen/lib/http/session/HTTPUpstreamSession.h>
#include <proxygen/lib/http/HTTPConnector.h>
#include <proxygen/httpserver/RequestHandler.h>
#include <proxygen/httpserver/RequestHandlerFactory.h>
#include <folly/ThreadLocal.h>
#include "BaseRequestHandler.h"

namespace restdbxx {
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
  void onDestroy(const proxygen::HTTPSession &) override {
    session_ = nullptr;
  }
};

class GiphySearchRequestHandler
    : public BaseRequestHandler,
      private proxygen::HTTPConnector::Callback {
 public:
  GiphySearchRequestHandler(const std::string &mount_path, const std::string &ak,
                            folly::HHWheelTimer *timer)
      : _mount_path(mount_path),
        _api_key(ak),
        _connector{this, timer},
        _serverHandler(*this) {}
  void onRequest(std::unique_ptr<proxygen::HTTPMessage> headers) noexcept override;
  void onBody(std::unique_ptr<folly::IOBuf> body) noexcept override;
  void onUpgrade(proxygen::UpgradeProtocol prot) noexcept override;
  void onEOM() noexcept override;
  void requestComplete() noexcept override;
  void onError(proxygen::ProxygenError err) noexcept override;
 private:
  void connectSuccess(proxygen::HTTPUpstreamSession *session) noexcept override;
  void connectError(const folly::AsyncSocketException &ex) noexcept override;

  /** functions called from ServerTxnHandler*/

  void detachServerTransaction() noexcept;

  void onServerHeadersComplete(std::unique_ptr<proxygen::HTTPMessage> msg) noexcept;
  void onServerBody(std::unique_ptr<folly::IOBuf> chain) noexcept;
  void onServerEOM() noexcept;
  void onServerError(const proxygen::HTTPException &error) noexcept;
  void onServerEgressPaused() noexcept;
  void onServerEgressResumed() noexcept;
 private:

  /**
   *
   */
  class ServerTxnHandler : public proxygen::HTTPTransactionHandler {
   public:
    void setTransaction(proxygen::HTTPTransaction *txn) noexcept override;
    void detachTransaction() noexcept override;
    void onHeadersComplete(std::unique_ptr<proxygen::HTTPMessage> msg) noexcept override;
    void onBody(std::unique_ptr<folly::IOBuf> chain) noexcept override;
    void onTrailers(std::unique_ptr<proxygen::HTTPHeaders> trailers) noexcept override;
    void onEOM() noexcept override;
    void onUpgrade(proxygen::UpgradeProtocol protocol) noexcept override;
    void onError(const proxygen::HTTPException &error) noexcept override;
    void onEgressPaused() noexcept override;
    void onEgressResumed() noexcept override;
    explicit ServerTxnHandler(GiphySearchRequestHandler &parent) : parent(parent) {}
   private:
    GiphySearchRequestHandler &parent;
  };

  ServerTxnHandler _serverHandler;
  std::string _api_key;
  proxygen::HTTPConnector _connector;
  std::unique_ptr<proxygen::HTTPMessage> _request;
  std::unique_ptr<SessionWrapper> _session;
  std::unique_ptr<folly::IOBuf> _giphyBody;
  proxygen::HTTPTransaction *_txn{nullptr};
  proxygen::HTTPMessage *_giphyRequest{nullptr};

  std::string _giphy_json_str;
  std::string _mount_path;
  enum GiphyOutputFormat {
    JSON = 0,
    HTML
  };
  GiphyOutputFormat output_format{JSON};
  std::string buildGiphyRequest();
  folly::Try<folly::dynamic> parseGiphyBody();
};

class GiphySearchRequestHandlerFactory : public proxygen::RequestHandlerFactory {
  std::string mount_path;
  std::string giphy_api_key;
  struct TimerWrapper {
    folly::HHWheelTimer::UniquePtr timer;
  };
  folly::ThreadLocal<TimerWrapper> timer_;
 public:
  GiphySearchRequestHandlerFactory(const std::string &mount_path);
  GiphySearchRequestHandlerFactory(const std::string &mount_path, const std::string &giphy_api_key);
  void onServerStart(folly::EventBase *evb) noexcept override;
  void onServerStop() noexcept override;
  proxygen::RequestHandler *onRequest(proxygen::RequestHandler *handler,
                                      proxygen::HTTPMessage *message) noexcept override;
};

}
#endif //RESTDBXX_GIPHYSEARCHREQUESTHANDLER_H
