//
// Created by victor on 5/11/17.
//

#ifndef RESTDBXX_CONFIGURATION_H
#define RESTDBXX_CONFIGURATION_H
#include <string>
#include <memory>
#include <folly/dynamic.h>
namespace restdbxx {


class RestDbConfiguration {
  int http_port;
  int https_port;
 public:
  int getHttps_port() const;
  void setHttps_port(int https_port);
 private:
  int spdy_port;
  int h2_port;
  std::string ip;
  int threads;
  std::string db_path;

  // file server endpoint;
  bool file_server_enabled = false;
  // endpoint path to access files
  std::string file_server_path = {};
  // filesystem directory served
  std::string file_server_root = {};

 public:
  bool is_file_server_enabled() const;
  void set_file_server_enabled(bool file_server_enabled);
  const std::string &getFile_server_path() const;
  void setFile_server_path(const std::string &file_server_path);
  const std::string &getFile_server_root() const;
  void setFile_server_root(const std::string &file_server_root);
  RestDbConfiguration() = default;
  virtual ~RestDbConfiguration() = default;

  void loadConfiguration(const std::string &path);
  void dumpConfiguration(const std::string &path);
  int getHttp_port() const;
  void setHttp_port(int http_port);
  int getSpdy_port() const;
  void setSpdy_port(int spdy_port);
  int getH2_port() const;
  void setH2_port(int h2_port);
  const std::string &getIp() const;
  void setIp(const std::string &ip);
  int getThreads() const;
  void setThreads(int threads);
  const std::string &getDb_path() const;
  void setDb_path(const std::string &db_path);

  static std::shared_ptr<RestDbConfiguration> get_instance();
  folly::dynamic buildJsonObject();
};

}

#endif //RESTDBXX_CONFIGURATION_H
