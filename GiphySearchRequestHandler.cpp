//
// Created by victor on 11/22/17.
//
#include "GiphySearchRequestHandler.h"
#include "Validations.h"
#include <glog/logging.h>
#include <proxygen/httpserver/ResponseBuilder.h>
#include <proxygen/lib/utils/URL.h>
#include <proxygen/lib/http/session/HTTPUpstreamSession.h>
#include <folly/io/async/EventBaseManager.h>
#include <boost/algorithm/string.hpp>
#include <folly/json.h>
#include <folly/SocketAddress.h>
#include <folly/futures/Promise.h>
#include <folly/executors/GlobalExecutor.h>

using std::string;
using proxygen::HTTPMethod;
using proxygen::HTTPMessage;
using proxygen::HTTPHeaders;
using proxygen::HTTPHeaderCode;
using proxygen::ResponseBuilder;
using folly::HHWheelTimer;

namespace {

class invalid_giphy_path : public std::domain_error {
 public:
  invalid_giphy_path() : domain_error("path for giphy endpoint is invalid") {}
};
}

namespace restdbxx {
void GiphySearchRequestHandler::onRequest(std::unique_ptr<HTTPMessage> headers) noexcept {
  using std::string;
  _request = std::move(headers);
  std::string urlStr = "unknown url";

  folly::SocketAddress addr; //(_giphyRequest->getDstAddress());
  try {
    urlStr = buildGiphyRequest();
    proxygen::URL url(urlStr);
    addr.setFromHostPort(url.getHost(), url.getPort());
  } catch (const invalid_giphy_path &e) {

    ResponseBuilder(downstream_)
        .status(404, "not found")
        .body(folly::to<string>("invalid path: ",
                                urlStr))
        .sendWithEOM();
    return;
  } catch (...) {
    ResponseBuilder(downstream_)
        .status(503, "Bad Gateway")
        .body(folly::to<string>("Could not parse server from URL: ",
                                urlStr))
        .sendWithEOM();
    return;
  }

//  downstream_->pauseIngress();
  VLOG(google::GLOG_INFO) << "Trying to connect to " << addr;
  auto evb = folly::EventBaseManager::get()->getEventBase();

  // A more sophisticated proxy would have a connection pool here
  const folly::AsyncSocket::OptionMap opts{
      {{SOL_SOCKET, SO_REUSEADDR}, 1}};
  downstream_->pauseIngress();
  _connector.connect(evb, addr,
                     std::chrono::milliseconds(1000),
                     opts);
}
string GiphySearchRequestHandler::buildGiphyRequest() {
  const static std::map<string, string> paths_map = {
      {"/search", "http://api.giphy.com/v1/gifs/search"},
      {"/trending", "http://api.giphy.com/v1/gifs/trending"},
      {"/random", "http://api.giphy.com/v1/gifs/random"}
  };

  string urlStr =
      "http://api.giphy.com/v1/gifs/search";
  std::string path = _request->getPath();
  boost::erase_first(path, _mount_path);
  auto found = paths_map.find(path);
  if (found != paths_map.end()) {
    urlStr = found->second;
  } else {
    throw invalid_giphy_path();
  }
  std::string q = _request->getDecodedQueryParam("q");
  int limit = _request->hasQueryParam("limit") ? _request->getIntQueryParam("limit", 100) : 100;
  const std::map<string, string> param_map = _request->getQueryParams();
  if (param_map.find("format") != param_map.end()) {
    std::string format = param_map.at("format");
    boost::to_upper(format);
    if (format == "HTML") {
      output_format = HTML;
    }
  }
  _giphyRequest = new HTTPMessage();
  _giphyRequest->setMethod(HTTPMethod::GET);
  _giphyRequest->setURL(urlStr);
  _giphyRequest->setQueryParam("api_key", _api_key);
  _giphyRequest->setQueryParam("limit", std::__cxx11::to_string(limit));
  _giphyRequest->setQueryParam("offset", "0");
  _giphyRequest->setQueryParam("rating", "R");
  _giphyRequest->setQueryParam("lang", "en");
  _giphyRequest->setQueryParam("q", q);
  return urlStr;
}
void GiphySearchRequestHandler::onBody(std::unique_ptr<folly::IOBuf> body) noexcept {
//  if (txn)
}
void GiphySearchRequestHandler::onUpgrade(proxygen::UpgradeProtocol prot) noexcept {

}
void GiphySearchRequestHandler::onEOM() noexcept {

}
void GiphySearchRequestHandler::requestComplete()noexcept {

}
void GiphySearchRequestHandler::onError(proxygen::ProxygenError err)noexcept {

}
void GiphySearchRequestHandler::connectSuccess(proxygen::HTTPUpstreamSession *session) noexcept {
  VLOG(google::INFO) << "Established " << *session;
  _session = std::make_unique<SessionWrapper>(session);
  _txn = session->newTransaction(&_serverHandler);
  LOG(INFO) << "Sending client request: " << _giphyRequest->getURL()
            << " to server";
  _giphyRequest->getHeaders().add(HTTPHeaderCode::HTTP_HEADER_ALLOW, "*");

  _giphyRequest->getHeaders().add(HTTPHeaderCode::HTTP_HEADER_ORIGIN, "localhost");
  _giphyRequest->getHeaders().add(HTTPHeaderCode::HTTP_HEADER_HOST, "api.giphy.com");
  _giphyRequest->dumpMessage(google::INFO);
  _txn->sendHeadersWithEOM(*_giphyRequest);
  downstream_->resumeIngress();
}
void GiphySearchRequestHandler::connectError(const folly::AsyncSocketException &ex) noexcept {
  VLOG(google::GLOG_INFO) << "connect error -- " << ex.what();
}
void GiphySearchRequestHandler::detachServerTransaction() noexcept {
  VLOG(google::GLOG_INFO) << "detached txn";

}
void GiphySearchRequestHandler::onServerHeadersComplete(std::unique_ptr<HTTPMessage> msg) noexcept {
//  CHECK(!clientTerminated_);
  VLOG(google::INFO) << "Forwarding " << msg->getStatusCode() << " response to client";
  if (output_format == JSON) downstream_->sendHeaders(*msg);
}
void GiphySearchRequestHandler::onServerBody(std::unique_ptr<folly::IOBuf> chain) noexcept {
//  CHECK(!clientTerminated_);
  LOG(INFO) << "Forwarding " <<
            ((chain) ? chain->computeChainDataLength() : 0) << " body bytes to client";
  if (output_format == JSON) {
    downstream_->sendBody(std::move(chain));
    return;
  }
  if (_giphyBody) {
    _giphyBody->prependChain(std::move(chain));
  } else {
    _giphyBody = std::move(chain);
  }
}

void GiphySearchRequestHandler::onServerEOM() noexcept {
  LOG(INFO) << "end of giphy response";
  if (output_format == HTML) {
    folly::Promise<folly::dynamic> promise;
    auto f = promise.getFuture();
    folly::EventBaseManager::get()->getEventBase()
        ->runInLoop([p = std::move(promise), this]() mutable {
          p.setTry(parseGiphyBody());
        });

    f.then([this](folly::dynamic &giphy_obj) {
      LOG(INFO) << folly::toPrettyJson(giphy_obj);
      std::stringstream ss;
      ss << "<html>"
         << "<head><title>" << "Giphy query"
         << "</title>"
         << "<script src=\"//unpkg.com/masonry-layout@4/dist/masonry.pkgd.min.js\"></script>"
         << "</head>"
         << "<body>";
      if (giphy_obj.find("data") != giphy_obj.items().end()) {
        folly::dynamic &data = giphy_obj.at("data");
        if (data.isArray()) {
          ss << "<div class=\"grid\" data-masonry='{ \"itemSelector\": \".grid-item\", \"columnWidth\": 200 }'>";
          for (auto &item: data) {
            ss << "<div class\"grid-item\">"
               << "<img src=\""
               << item.at("images").at("downsized").at("url")
               << "\" >"
               << "</div>";
          }
          ss << "</div>";
        }
      }
      ss << "</body></html>";
      return ss.str();
    }).then([this](string &html) {

      ResponseBuilder(downstream_)
          .status(200, "OK")
          .body(std::move(html))
          .sendWithEOM();
    });
    //auto giphy_obj = folly::parseJson(_giphy_json_str);

  }
}
void GiphySearchRequestHandler::onServerError(const proxygen::HTTPException &error) noexcept {
  VLOG(google::GLOG_INFO) << "server error";

  ResponseBuilder(downstream_)
      .status(500, "internal error")
      .body(error.describe())
      .sendWithEOM();
}
void GiphySearchRequestHandler::onServerEgressPaused() noexcept {

}
void GiphySearchRequestHandler::onServerEgressResumed() noexcept {

}
folly::Try<folly::dynamic> GiphySearchRequestHandler::parseGiphyBody() {
  auto f = [b = _giphyBody->cloneCoalesced()]() {
    if (!b) {
      throw std::runtime_error("empty giphy body");
    }
    std::string piece(reinterpret_cast<const char *>(b->data()), b->length());
    return folly::parseJson(piece);
  };
  return folly::makeTryWith(f);
}
void GiphySearchRequestHandler::ServerTxnHandler::setTransaction(proxygen::HTTPTransaction *txn) noexcept {

}
void GiphySearchRequestHandler::ServerTxnHandler::detachTransaction() noexcept {
  parent.detachServerTransaction();
}
void GiphySearchRequestHandler::ServerTxnHandler::onHeadersComplete(std::unique_ptr<proxygen::HTTPMessage> msg) noexcept {
  parent.onServerHeadersComplete(std::move(msg));
}
void GiphySearchRequestHandler::ServerTxnHandler::onBody(std::unique_ptr<folly::IOBuf> chain) noexcept {
  parent.onServerBody(std::move(chain));
}
void GiphySearchRequestHandler::ServerTxnHandler::onTrailers(std::unique_ptr<proxygen::HTTPHeaders> trailers) noexcept {
//  parent.onser
}
void GiphySearchRequestHandler::ServerTxnHandler::onEOM() noexcept {
  parent.onServerEOM();
}
void GiphySearchRequestHandler::ServerTxnHandler::onUpgrade(proxygen::UpgradeProtocol protocol) noexcept {

}
void GiphySearchRequestHandler::ServerTxnHandler::onError(const proxygen::HTTPException &error) noexcept {
  parent.onServerError(error);
}
void GiphySearchRequestHandler::ServerTxnHandler::onEgressPaused() noexcept {
  parent.onServerEgressPaused();
}
void GiphySearchRequestHandler::ServerTxnHandler::onEgressResumed() noexcept {
  parent.onServerEgressPaused();
}
void GiphySearchRequestHandlerFactory::onServerStart(folly::EventBase *evb) noexcept {
  timer_->timer = HHWheelTimer::newTimer(
      evb,
      std::chrono::milliseconds(HHWheelTimer::DEFAULT_TICK_INTERVAL),
      folly::AsyncTimeout::InternalEnum::NORMAL,
      std::chrono::seconds(1));
}
void GiphySearchRequestHandlerFactory::onServerStop() noexcept {
  timer_->timer.reset();
}
proxygen::RequestHandler *GiphySearchRequestHandlerFactory::onRequest(proxygen::RequestHandler *handler,
                                                                      HTTPMessage *message) noexcept {
  std::string path = message->getPath();
  Validations::sanitize_path(path);
  if (boost::algorithm::starts_with(path, mount_path)) {
    return new GiphySearchRequestHandler(mount_path, giphy_api_key, timer_->timer.get());
  }
  return handler;
}
GiphySearchRequestHandlerFactory::GiphySearchRequestHandlerFactory(const string &mount_path) : mount_path(mount_path) {}
GiphySearchRequestHandlerFactory::GiphySearchRequestHandlerFactory(const string &mount_path,
                                                                   const string &giphy_api_key)
    : mount_path(mount_path), giphy_api_key(giphy_api_key) {}
}