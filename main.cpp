#include <unistd.h>
#include <string>

#include <iostream>
#include <folly/init/Init.h>
#include <gflags/gflags.h>
#include <folly/io/async/EventBaseManager.h>
#include <proxygen/httpserver/HTTPServer.h>
#include "includes.h"
#include "EndpointControllerFactory.h"
#include "FiltersFactory.h"
#include "UserManager.h"
#include "AuthenticationRequestHandler.h"
#include "FileServerRequestHandler.h"
using proxygen::HTTPServer;


using folly::EventBase;
using folly::EventBaseManager;
using folly::SocketAddress;



using Protocol = HTTPServer::Protocol;

DEFINE_int32(http_port, 11000, "Port to listen on with HTTP protocol");
DEFINE_int32(https_port, 11043, "Port to listen on with HTTPS protocol");
DEFINE_int32(spdy_port, 11001, "Port to listen on with SPDY protocol");
DEFINE_int32(h2_port, 11002, "Port to listen on with HTTP/2 protocol");
DEFINE_string(ip, "localhost", "IP/Hostname to bind to");
DEFINE_int32(threads, 16, "Number of threads to listen on. Numbers <= 0 "
    "will use the number of cores on this machine.");
DEFINE_string(db_path, "/tmp/restdb", "Path to the database");
DEFINE_string(ssl_cert, "cert.pem", "SSL certificate file");
DEFINE_string(ssl_key, "key.pem", "SSL key file");

using google::GLOG_INFO;
void initConfiguration();
void init_services();
int main(int argc, char **argv) {
  using namespace proxygen;

  folly::init(&argc, &argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InstallFailureSignalHandler();

  std::vector<HTTPServer::IPConfig> IPs = {{SocketAddress(FLAGS_ip, FLAGS_https_port, true), Protocol::HTTP},
                                           {SocketAddress(FLAGS_ip, FLAGS_http_port, true), Protocol::HTTP},
                                           {SocketAddress(FLAGS_ip, FLAGS_spdy_port, true), Protocol::SPDY},
                                           {SocketAddress(FLAGS_ip, FLAGS_h2_port, true), Protocol::HTTP2},
  };
  wangle::SSLContextConfig sslConfig;
  sslConfig.certificates = {
      wangle::SSLContextConfig::CertificateInfo(FLAGS_ssl_cert, FLAGS_ssl_key, "")
  };
  sslConfig.isDefault = true;
  IPs.front().sslConfigs = {std::move(sslConfig)};
  sslConfig.isLocalPrivateKey = true;
  VLOG(GLOG_INFO) << "Starting restdbxx";
  VLOG(GLOG_INFO) << "HTTP PORT: " << FLAGS_http_port;
  VLOG(GLOG_INFO) << "HTTPS PORT: " << FLAGS_https_port;
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
  options.enableContentCompression = false;
  options.handlerFactories = RequestHandlerChain()
      .addThen<restdbxx::FiltersFactory>()
      .addThen<restdbxx::AuthenticationRequestHandlerFactory>()
      .addThen<restdbxx::EndpointControllerFactory>()
      .addThen<restdbxx::FileServerRequestHandlerFactory>("/files", "/home/victor/tmp")
      .addThen<restdbxx::UserRequestHandlerFactory>()
      .addThen<restdbxx::RestDbRequestHandlerFactory>()
      .build();
  options.h2cEnabled = false;

  proxygen::HTTPServer server(std::move(options));
  server.bind(IPs);

  init_services();

  // Start HTTPServer mainloop in a separate thread
  std::thread t([&]() {
    folly::setThreadName("Server main thread");
    server.start();
  });

  t.join();
  return 0;
}
const string &DEFAULT_ADMIN_PASSWORD() {
  static const std::string value = "admin";
  return value;
}
void init_services() {
  // force db init
  auto db = restdbxx::DbManager::get_instance();

  auto user_manager = restdbxx::UserManager::get_instance();

  if (!user_manager->user_exists("admin")) {
    VLOG(google::GLOG_INFO) << "admin user does not exist, creating";
    user_manager->create_user("admin", DEFAULT_ADMIN_PASSWORD());
    VLOG(google::GLOG_INFO) << "admin user created";
  } else {
    VLOG(google::GLOG_INFO) << "admin user exists";
  }
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

