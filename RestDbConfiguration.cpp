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
static const char *const GIPHY_API_KEY = "giphy_api_key";
static const char *const FILE_SERVER_ROOT = "file_server_root";
static const char *const FILE_SERVER_PATH = "file_server_path";
static const char *const FILE_SERVER_ENABLED = "file_server_enabled";
static const char *const DB_PATH = "db_path";
static const char *const THREADS = "threads";
static const char *const H2_PORT = "h2_port";
static const char *const SPDY_PORT = "spdy_port";
static const char *const HTTPS_PORT = "https_port";
static const char *const HTTP_PORT = "http_port";
static const char *const IP = "ip";
static const char *const GIPHY_MOUNT_PATH = "giphy_mount_path";
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
    if (conf_obj.find(IP) != conf_obj.items().end()) {
      ip = conf_obj[IP].asString();
    }
    if (conf_obj.find(HTTP_PORT) != conf_obj.items().end()) {
      http_port = conf_obj[HTTP_PORT].asInt();
    }
    if (conf_obj.find(HTTPS_PORT) != conf_obj.items().end()) {
      https_port = conf_obj[HTTPS_PORT].asInt();
    }
    if (conf_obj.find(SPDY_PORT) != conf_obj.items().end()) {
      spdy_port = conf_obj[SPDY_PORT].asInt();
    }
    if (conf_obj.find(H2_PORT) != conf_obj.items().end()) {
      h2_port = conf_obj[H2_PORT].asInt();
    }
    if (conf_obj.find(THREADS) != conf_obj.items().end()) {
      threads = conf_obj[THREADS].asInt();
    }
    if (conf_obj.find(DB_PATH) != conf_obj.items().end()) {
      db_path = conf_obj[DB_PATH].asString();
    }
    if (conf_obj.find(FILE_SERVER_ENABLED) != conf_obj.items().end()) {
      VLOG(google::GLOG_INFO)
      << "settings file_server_enabled from config file: " << conf_obj[FILE_SERVER_ENABLED].asBool();
      file_server_enabled = conf_obj[FILE_SERVER_ENABLED].asBool();
    }

    if (conf_obj.find(FILE_SERVER_PATH) != conf_obj.items().end()) {
      file_server_path = conf_obj[FILE_SERVER_PATH].asString();
    }

    if (conf_obj.find(FILE_SERVER_ROOT) != conf_obj.items().end()) {
      file_server_root = conf_obj[FILE_SERVER_ROOT].asString();
    }
    if (conf_obj.find(GIPHY_API_KEY) != conf_obj.items().end()) {
      giphy_api_key = conf_obj[GIPHY_API_KEY].asString();
    }
    if (conf_obj.find(GIPHY_MOUNT_PATH) != conf_obj.items().end()) {
      giphy_mount_path = conf_obj[GIPHY_MOUNT_PATH].asString();
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
  folly::dynamic res = folly::dynamic::object(HTTP_PORT, http_port)
      (HTTPS_PORT, https_port)
      (SPDY_PORT, spdy_port)
      (DB_PATH, db_path)
      (H2_PORT, h2_port)
      (IP, ip)
      (THREADS, threads)
      (FILE_SERVER_ENABLED, file_server_enabled)
      (FILE_SERVER_PATH, file_server_path)
      (FILE_SERVER_ROOT, file_server_root)
      (GIPHY_API_KEY, giphy_api_key)
      (GIPHY_MOUNT_PATH, giphy_mount_path);
  return res;
}
const std::string &RestDbConfiguration::get_giphy_api_key() const {
  return giphy_api_key;
}
void RestDbConfiguration::set_giphy_api_key(const std::string &giphy_api_key) {
  RestDbConfiguration::giphy_api_key = giphy_api_key;
}
const std::string &RestDbConfiguration::get_giphy_mount_path() const {
  return giphy_mount_path;
}
void RestDbConfiguration::set_giphy_mount_path(const std::string &giphy_mount_path) {
  RestDbConfiguration::giphy_mount_path = giphy_mount_path;
}

}