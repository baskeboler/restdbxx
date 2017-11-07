//
// Created by victor on 5/11/17.
//

#include "RestDbConfiguration.h"

#include <folly/Singleton.h>

namespace restdbxx {

namespace {
struct RestDbConfigurationTag {};
folly::Singleton<RestDbConfiguration, RestDbConfigurationTag> the_instance;
}

std::shared_ptr<RestDbConfiguration> RestDbConfiguration::get_instance() {
  return the_instance.try_get();
}


int RestDbConfiguration::getHttp_port() const {
  return http_port;
}
void RestDbConfiguration::setHttp_port(int http_port) {
  RestDbConfiguration::http_port = http_port;
}
int RestDbConfiguration::getSpdy_port() const {
  return spdy_port;
}
void RestDbConfiguration::setSpdy_port(int spdy_port) {
  RestDbConfiguration::spdy_port = spdy_port;
}
int RestDbConfiguration::getH2_port() const {
  return h2_port;
}
void RestDbConfiguration::setH2_port(int h2_port) {
  RestDbConfiguration::h2_port = h2_port;
}
const std::string &RestDbConfiguration::getIp() const {
  return ip;
}
void RestDbConfiguration::setIp(const std::string &ip) {
  RestDbConfiguration::ip = ip;
}
int RestDbConfiguration::getThreads() const {
  return threads;
}
void RestDbConfiguration::setThreads(int threads) {
  RestDbConfiguration::threads = threads;
}
const std::string &RestDbConfiguration::getDb_path() const {
  return db_path;
}
void RestDbConfiguration::setDb_path(const std::string &db_path) {
  RestDbConfiguration::db_path = db_path;
}

}