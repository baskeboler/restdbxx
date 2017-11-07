#include <unistd.h>
#include <string>

#include <iostream>
#include <folly/init/Init.h>
#include <gflags/gflags.h>
#include <folly/io/async/EventBaseManager.h>
#include <proxygen/httpserver/HTTPServer.h>
#include "includes.h"

using proxygen::HTTPServer;


using folly::EventBase;
using folly::EventBaseManager;
using folly::SocketAddress;



using Protocol = HTTPServer::Protocol;


DEFINE_int32(http_port, 11000, "Port to listen on with HTTP protocol");
DEFINE_int32(spdy_port, 11001, "Port to listen on with SPDY protocol");
DEFINE_int32(h2_port, 11002, "Port to listen on with HTTP/2 protocol");
DEFINE_string(ip, "localhost", "IP/Hostname to bind to");
DEFINE_int32(threads, 16, "Number of threads to listen on. Numbers <= 0 "
    "will use the number of cores on this machine.");
DEFINE_string(db_path, "/tmp/restdb", "Path to the database");

using google::GLOG_INFO;
void initConfiguration();
int main(int argc, char **argv) {
  using namespace proxygen;

  folly::init(&argc, &argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InstallFailureSignalHandler();

  std::vector<HTTPServer::IPConfig> IPs = {{SocketAddress(FLAGS_ip, FLAGS_http_port, true), Protocol::HTTP},
                                           {SocketAddress(FLAGS_ip, FLAGS_spdy_port, true), Protocol::SPDY},
                                           {SocketAddress(FLAGS_ip, FLAGS_h2_port, true), Protocol::HTTP2},
  };
  VLOG(GLOG_INFO) << "Starting restdbxx";
  VLOG(GLOG_INFO) << "HTTP PORT: " << FLAGS_http_port;
  VLOG(GLOG_INFO) << "SPDY PORT: " << FLAGS_spdy_port;
  VLOG(GLOG_INFO) << "H2 PORT: " << FLAGS_h2_port;

  if (FLAGS_threads <= 0) {
    FLAGS_threads = sysconf(_SC_NPROCESSORS_ONLN);
    CHECK(FLAGS_threads > 0);
  }
  initConfiguration();

  proxygen::HTTPServerOptions options;
  options.threads = static_cast<size_t>(FLAGS_threads);
  options.idleTimeout = std::chrono::milliseconds(600);
  options.shutdownOn = {SIGINT, SIGTERM};
  options.enableContentCompression = true;
  options.handlerFactories = RequestHandlerChain()
      .addThen<restdbxx::UserRequestHandlerFactory>()
      .addThen<restdbxx::RestDbRequestHandlerFactory>()
      .build();
  options.h2cEnabled = false;

  proxygen::HTTPServer server(std::move(options));
  server.bind(IPs);


  // Start HTTPServer mainloop in a separate thread
  std::thread t([&]() {
    folly::setThreadName("Server main thread");
    server.start();
  });

  t.join();
  return 0;
}

void initConfiguration() {
  std::shared_ptr<restdbxx::RestDbConfiguration> config = restdbxx::RestDbConfiguration::get_instance();
  config->setHttp_port(FLAGS_http_port);
  config->setSpdy_port(FLAGS_spdy_port);
  config->setDb_path(FLAGS_db_path);
  config->setH2_port(FLAGS_h2_port);
  config->setIp(FLAGS_ip);
  config->setThreads(FLAGS_threads);
}
