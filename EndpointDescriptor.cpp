//
// Created by victor on 11/8/17.
//

#include "EndpointDescriptor.h"


namespace restdbxx {

long EndpointDescriptor::getId() const {
  return id;
}
void EndpointDescriptor::setId(long id) {
  EndpointDescriptor::id = id;
}
const std::string &EndpointDescriptor::getUrl() const {
  return url;
}
void EndpointDescriptor::setUrl(const std::string &url) {
  EndpointDescriptor::url = url;
}
int EndpointDescriptor::getCount() const {
  return count;
}
void EndpointDescriptor::setCount(int count) {
  EndpointDescriptor::count = count;
}
const ptime &EndpointDescriptor::getCreated() const {
  return created;
}
void EndpointDescriptor::setCreated(const ptime &created) {
  EndpointDescriptor::created = created;
}
const ptime &EndpointDescriptor::getModified() const {
  return modified;
}
void EndpointDescriptor::setModified(const ptime &modified) {
  EndpointDescriptor::modified = modified;
}
const std::string &EndpointDescriptor::getModification_user() const {
  return modification_user;
}
void EndpointDescriptor::setModification_user(const std::string &modification_user) {
  EndpointDescriptor::modification_user = modification_user;
}
bool EndpointDescriptor::isEnabled() const {
  return enabled;
}
void EndpointDescriptor::setEnabled(bool enabled) {
  EndpointDescriptor::enabled = enabled;
}
std::shared_ptr<EndpointDescriptor> EndpointDescriptor::new_endpoint(long id, const std::string &url, const std::string &user) {
  return std::make_shared<EndpointDescriptor>(id, url, user);
}

EndpointDescriptor::EndpointDescriptor(long id,
                                       const std::string &url,
                                       const std::string &modification_user,
                                       int count,
                                       const ptime &created,
                                       const ptime &modified,
                                       bool enabled
)
    : id(id),
      url(url),
      count(count),
      created(created),
      modified(modified),
      modification_user(modification_user),
      enabled(enabled) {}
std::shared_ptr<EndpointDescriptor> EndpointDescriptor::fromDynamic(folly::dynamic &obj) {
  if (obj.isObject()) {
    return std::make_shared<EndpointDescriptor>(obj.at("id").asInt(),
                                                obj.at("url").asString(),
                                                obj["modification_user"].asString(),
                                                obj["count"].asInt(),
                                                from_iso_extended_string(obj["created"].asString()),
                                                from_iso_extended_string(obj["modified"].asString()),
                                                obj["enabled"].asBool());
  } else {
    BOOST_THROW_EXCEPTION(std::invalid_argument("not a json object"));
  }
}
}