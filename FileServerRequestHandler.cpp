//
// Created by victor on 11/17/17.
//

#include <folly/File.h>

#include <folly/FileUtil.h>
#include <folly/executors/GlobalExecutor.h>
#include <folly/io/async/EventBaseManager.h>
#include <proxygen/httpserver/ResponseBuilder.h>
#include <proxygen/httpserver/RequestHandler.h>
#include <boost/algorithm/string.hpp>
#include "FileServerRequestHandler.h"
#include <boost/filesystem.hpp>

using proxygen::ResponseBuilder;
using proxygen::HTTPMethod;
namespace restdbxx {

void FileServerRequestHandler::onRequest(std::unique_ptr<proxygen::HTTPMessage> headers) noexcept {
  if (headers->getMethod() != HTTPMethod::GET) {
    ResponseBuilder(downstream_)
        .status(400, "Bad method")
        .body("Only GET is supported")
        .sendWithEOM();
    return;
  }
  // a real webserver would validate this path didn't contain malicious
  // characters like '//' or '..'
  try {
    // + 1 to kill leading /
    std::string path = headers->getPath();
    boost::algorithm::erase_first(path, path_prefix);
    real_path = root + path;
    file_ = std::make_unique<folly::File>(real_path.c_str());
  } catch (const std::system_error &ex) {
    ResponseBuilder(downstream_)
        .status(404, "Not Found")
        .body(folly::to<std::string>("Could not find ", headers->getPath(),
                                     " ex=", folly::exceptionStr(ex)))
        .sendWithEOM();
    return;
  }
  ResponseBuilder(downstream_)
      .status(200, "Ok")
      .send();
  bool is_dir = boost::filesystem::is_directory(real_path);
  if (is_dir) {
    readFileScheduled_ = true;
    folly::getCPUExecutor()->add(
        std::bind(&FileServerRequestHandler::listFiles, this,
                  folly::EventBaseManager::get()->getEventBase())
    );
    return;
  }
  //if (file_)
  // use a CPU executor since read(2) of a file can block
  readFileScheduled_ = true;
  folly::getCPUExecutor()->add(
      std::bind(&FileServerRequestHandler::readFile, this,
                folly::EventBaseManager::get()->getEventBase()));
}
void FileServerRequestHandler::onBody(std::unique_ptr<folly::IOBuf> body) noexcept {

}
void FileServerRequestHandler::onUpgrade(proxygen::UpgradeProtocol prot) noexcept {

}
void FileServerRequestHandler::onEOM() noexcept {

}
void FileServerRequestHandler::requestComplete()noexcept {
  finished_ = true;
  paused_ = true;
  checkForCompletion();

}
void FileServerRequestHandler::onError(proxygen::ProxygenError err) noexcept {
  finished_ = true;
  paused_ = true;
  checkForCompletion();

}
void FileServerRequestHandler::readFile(folly::EventBase *evb) {
  folly::IOBufQueue buf;
  while (file_ && !paused_) {
    // read 4k-ish chunks and foward each one to the client
    auto data = buf.preallocate(4000, 4000);
    auto rc = folly::readNoInt(file_->fd(), data.first, data.second);
    if (rc < 0) {
      // error
      VLOG(4) << "Read error=" << rc;
      file_.reset();
      evb->runInEventBaseThread([this] {
        LOG(ERROR) << "Error reading file";
        downstream_->sendAbort();
      });
      break;
    } else if (rc == 0) {
      // done
      file_.reset();
      VLOG(4) << "Read EOF";
      evb->runInEventBaseThread([this] {
        ResponseBuilder(downstream_)
            .sendWithEOM();
      });
      break;
    } else {
      buf.postallocate(rc);
      evb->runInEventBaseThread([this, body = buf.move()]() mutable {
        ResponseBuilder(downstream_)
            .body(std::move(body))
            .send();
      });
    }
  }

  // Notify the request thread that we terminated the readFile loop
  evb->runInEventBaseThread([this] {
    readFileScheduled_ = false;
    if (!checkForCompletion() && !paused_) {
      VLOG(4) << "Resuming deferred readFile";
      onEgressResumed();
    }
  });
}
bool FileServerRequestHandler::checkForCompletion() {
  if (finished_ && !readFileScheduled_) {
    VLOG(4) << "deleting StaticHandler";
    delete this;
    return true;
  }
  return false;
}
void FileServerRequestHandler::onEgressPaused() noexcept {
// This will terminate readFile soon
  VLOG(4) << "FileServerRequestHandler paused";
  paused_ = true;

}
void FileServerRequestHandler::onEgressResumed() noexcept {
  VLOG(4) << "FileServerRequestHandler resumed";
  paused_ = false;
  // If readFileScheduled_, it will reschedule itself
  if (!readFileScheduled_ && file_) {
    readFileScheduled_ = true;
    folly::getCPUExecutor()->add(
        std::bind(&FileServerRequestHandler::readFile, this,
                  folly::EventBaseManager::get()->getEventBase()));
  } else {
    VLOG(4) << "Deferred scheduling readFile";
  }
}
FileServerRequestHandler::FileServerRequestHandler(const std::string &path_prefix, const std::string &root)
    : path_prefix(path_prefix), root(root) {}

void FileServerRequestHandler::listFiles(folly::EventBase *evb) {
  namespace fs = boost::filesystem;
  fs::path p(real_path);
  std::stringstream ss;
  ss << "<html>"
      "<head>"
      "<title>file list</title>"
      "</head>"
      "<body>"
      "<h1>Listing for " << real_path << "</h1>"
         "<ul>";
  for (fs::directory_entry &entry: fs::directory_iterator(p)) {
    ss << "<li>" << entry.path() << "</li>";
  }
  ss << "</ul>"
      "</body>"
      "</html>";

  std::string html = ss.str();

  evb->runInEventBaseThread([data = std::move(html), this]() mutable {
    proxygen::ResponseBuilder(downstream_)
        .body(std::move(data))
        .sendWithEOM();
    finished_ = true;
  });

}

void FileServerRequestHandlerFactory::onServerStart(folly::EventBase *evb) noexcept {

}
void FileServerRequestHandlerFactory::onServerStop() noexcept {

}
proxygen::RequestHandler *FileServerRequestHandlerFactory::onRequest(proxygen::RequestHandler *handler,
                                                                     proxygen::HTTPMessage *message) noexcept {
  if (boost::algorithm::starts_with(message->getPath(), path_prefix))
    return new FileServerRequestHandler(path_prefix, root);
  return handler;
}
FileServerRequestHandlerFactory::FileServerRequestHandlerFactory(const std::string &path_prefix,
                                                                 const std::string &root)
    : path_prefix(path_prefix), root(root) {}
}