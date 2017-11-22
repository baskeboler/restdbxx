//
// Created by victor on 5/11/17.
//

#include "RestDbConfiguration.h"

#include <folly/Singleton.h>
#include <folly/dynamic.h>
#include <folly/File.h>
#include <boost/filesystem.hpp>
#include <folly/json.h>
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
void RestDbConfiguration::loadConfiguration(const std::string &path) {

  folly::dynamic conf_obj;
  namespace fs = boost::filesystem;
  bool file_exists = fs::exists(path);
  if (file_exists) {
    fs::ifstream is;
    is.open(path);
    std::stringstream ss;
    while (!is.eof()) {
      std::string line;
      is >> line;
      ss << line;
    }
    is.close();
    conf_obj = folly::parseJson(ss.str());
    if (conf_obj.find("ip") != conf_obj.items().end()) {
      ip = conf_obj["ip"].asString();
    }
    if (conf_obj.find("http_port") != conf_obj.items().end()) {
      http_port = conf_obj["http_port"].asInt();
    }
    if (conf_obj.find("https_port") != conf_obj.items().end()) {
      https_port = conf_obj["https_port"].asInt();
    }
    if (conf_obj.find("spdy_port") != conf_obj.items().end()) {
      spdy_port = conf_obj["spdy_port"].asInt();
    }
    if (conf_obj.find("h2_port") != conf_obj.items().end()) {
      h2_port = conf_obj["h2_port"].asInt();
    }
    if (conf_obj.find("threads") != conf_obj.items().end()) {
      threads = conf_obj["threads"].asInt();
    }
    if (conf_obj.find("db_path") != conf_obj.items().end()) {
      db_path = conf_obj["db_path"].asString();
    }
    if (conf_obj.find("file_server_enabled") != conf_obj.items().end()) {
      VLOG(google::GLOG_INFO)
      << "settings file_server_enabled from config file: " << conf_obj["file_server_enabled"].asBool();
      file_server_enabled = conf_obj["file_server_enabled"].asBool();
    }

    if (conf_obj.find("file_server_path") != conf_obj.items().end()) {
      file_server_path = conf_obj["file_server_path"].asString();
    }

    if (conf_obj.find("file_server_root") != conf_obj.items().end()) {
      file_server_root = conf_obj["file_server_root"].asString();
    }
  }
}
bool RestDbConfiguration::is_file_server_enabled() const {
  return file_server_enabled;
}
void RestDbConfiguration::set_file_server_enabled(bool file_server_enabled) {
  RestDbConfiguration::file_server_enabled = file_server_enabled;
}
const std::string &RestDbConfiguration::getFile_server_path() const {
  return file_server_path;
}
void RestDbConfiguration::setFile_server_path(const std::string &file_server_path) {
  RestDbConfiguration::file_server_path = file_server_path;
}
const std::string &RestDbConfiguration::getFile_server_root() const {
  return file_server_root;
}
void RestDbConfiguration::setFile_server_root(const std::string &file_server_root) {
  RestDbConfiguration::file_server_root = file_server_root;
}
int RestDbConfiguration::getHttps_port() const {
  return https_port;
}
void RestDbConfiguration::setHttps_port(int https_port) {
  RestDbConfiguration::https_port = https_port;
}
void RestDbConfiguration::dumpConfiguration(const std::string &path) {
  folly::dynamic conf_obj = buildJsonObject();
  std::ofstream os(path);
  os << folly::toPrettyJson(conf_obj);
  os.close();
}

folly::dynamic RestDbConfiguration::buildJsonObject() {
  folly::dynamic res = folly::dynamic::object("http_port", http_port)
      ("https_port", https_port)
      ("spdy_port", spdy_port)
      ("db_path", db_path)
      ("h2_port", h2_port)
      ("ip", ip)
      ("threads", threads)
      ("file_server_enabled", file_server_enabled)
      ("file_server_path", file_server_path)
      ("file_server_root", file_server_root);
  return res;
}

}