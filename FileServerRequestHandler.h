//
// Created by victor on 11/17/17.
//

#ifndef RESTDBXX_FILESERVERREQUESTHANDLER_H
#define RESTDBXX_FILESERVERREQUESTHANDLER_H

#include <proxygen/httpserver/RequestHandler.h>
#include <proxygen/httpserver/RequestHandlerFactory.h>
namespace restdbxx {
class FileServerRequestHandler : public proxygen::RequestHandler {
 public:
  FileServerRequestHandler(const std::string &path_prefix, const std::string &root);
  void onRequest(std::unique_ptr<proxygen::HTTPMessage> headers)noexcept override;
  void onBody(std::unique_ptr<folly::IOBuf> body) noexcept override;
  void onUpgrade(proxygen::UpgradeProtocol prot) noexcept override;
  void onEOM() noexcept override;
  void requestComplete() noexcept override;
  void onError(proxygen::ProxygenError err) noexcept override;
  void onEgressPaused() noexcept override;
  void onEgressResumed()noexcept override;
 private:
  void readFile(folly::EventBase *evb);
  bool checkForCompletion();
  std::string path_prefix;
  std::string root;
  std::unique_ptr<folly::File> file_;
  bool readFileScheduled_{false};
  std::atomic<bool> paused_{false};
  bool finished_{false};
  std::string real_path;
  void listFiles(folly::EventBase *evb);
};

class FileServerRequestHandlerFactory : public proxygen::RequestHandlerFactory {
 private:
  std::string path_prefix;
  std::string root;
 public:
  FileServerRequestHandlerFactory(const std::string &path_prefix, const std::string &root);
  void onServerStart(folly::EventBase *evb) noexcept override;
  FileServerRequestHandlerFactory(FileServerRequestHandlerFactory &other)
      : FileServerRequestHandlerFactory(other.path_prefix, other.root) {

  }
  void onServerStop() noexcept override;
  proxygen::RequestHandler *onRequest(proxygen::RequestHandler *handler,
                                      proxygen::HTTPMessage *message) noexcept override;
};

}

#endif //RESTDBXX_FILESERVERREQUESTHANDLER_H
