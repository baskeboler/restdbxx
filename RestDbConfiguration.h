//
// Created by victor on 5/11/17.
//

#ifndef RESTDBXX_CONFIGURATION_H
#define RESTDBXX_CONFIGURATION_H
#include <string>
#include <memory>
namespace restdbxx {


class RestDbConfiguration {
  int http_port;
  int spdy_port;
  int h2_port;
  std::string ip;
  int threads;
  std::string db_path;

 public:
  RestDbConfiguration() = default;
  virtual ~RestDbConfiguration() = default;

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
};

}

#endif //RESTDBXX_CONFIGURATION_H
