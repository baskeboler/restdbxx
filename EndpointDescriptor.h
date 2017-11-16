//
// Created by victor on 11/8/17.
//

#ifndef RESTDBXX_ENDPOINTDESCRIPTOR_H
#define RESTDBXX_ENDPOINTDESCRIPTOR_H
#include <string>
#include <vector>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/time_formatters.hpp>
#include <folly/dynamic.h>
#include <stdexcept>
#include <boost/throw_exception.hpp>
using namespace boost::posix_time;
using namespace boost::gregorian;

namespace restdbxx {
class EndpointDescriptor {
 public:
  EndpointDescriptor(long id,
                     const std::string &url,
                     const std::string &modification_user,
                     int count = 0,
                     const ptime &created = second_clock::local_time(),
                     const ptime &modified = second_clock::local_time(),
                     bool enabled = true);

  long getId() const;
  void setId(long id);
  const std::string &getUrl() const;
  void setUrl(const std::string &url);
  int getCount() const;
  void setCount(int count);
  const ptime &getCreated() const;
  void setCreated(const ptime &created);
  const ptime &getModified() const;
  void setModified(const ptime &modified);
  const std::string &getModification_user() const;
  void setModification_user(const std::string &modification_user);
  bool isEnabled() const;
  void setEnabled(bool enabled);

  folly::dynamic getDynamic() const;
  static std::shared_ptr<EndpointDescriptor> new_endpoint(long id, const std::string &url, const std::string &user);
  static std::shared_ptr<EndpointDescriptor> fromDynamic(folly::dynamic &obj);

 private:
  long id;
  std::string url;
  int count;
  ptime created;
  ptime modified;
  std::string modification_user;
  bool enabled;
};

}

#endif //RESTDBXX_ENDPOINTDESCRIPTOR_H
