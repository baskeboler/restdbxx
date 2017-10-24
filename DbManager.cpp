//
// Created by victor on 23/10/17.
//

#include "DbManager.h"
#include <iterator>
#include <folly/Singleton.h>
#include <folly/dynamic-inl.h>

namespace restdbxx    {
namespace {
struct DbManagerSingletonTag {};
}

static folly::Singleton<DbManager, DbManagerSingletonTag> the_instance;

DbManager::DbManager(): _root(folly::dynamic::object()) {

  //_root = folly::dynamic::object();

  //_root.
}
bool DbManager::path_exists(const std::string &path) {
  std::vector<std::string> tokens;

  boost::split(tokens, path, boost::is_any_of("/"), boost::token_compress_on);
  folly::dynamic *aux = &_root;
  for (auto part =tokens.begin(); part != tokens.end(); ++part) {
    if ((*part).length() == 0) continue;
    LOG(INFO) << "Checking part " << *part;

    if (!aux->isObject()) {
      return false;
    }
    if (aux->find(*part) == aux->items().end()) {
      return false;
    } else if (std::distance(part, tokens.end()) == 1) {
      // last token
      return true;
    } else {

      aux  = & aux->at(*part);
    }
  }
  return true;
}
folly::Optional<folly::dynamic> DbManager::get_path(const std::string &path) {
  std::vector<std::string> tokens = get_path_parts(path);
  folly::dynamic *aux = &_root;
  for (const auto &part: tokens) {
    auto n = aux->find(part);
    if (n == aux->items().end()) {
      return folly::none;
    }
    aux = & n->second;
    //aux = aux[part];
  }
  return *aux;
}
std::vector<std::string> DbManager::get_path_parts(const std::string &path) const {
  std::vector<std::__cxx11::string> tokens;
  std::string path_copy = path;
  boost::erase_first(path_copy, "/");
  split(tokens, path_copy, boost::is_any_of("/"));
  return tokens;
}
folly::dynamic DbManager::to_deep_object(std::vector<std::string> &path, const folly::dynamic &unwrapped) {
  if (path.empty())
    return unwrapped;
  std::string last = *path.rbegin();
  path.pop_back();
  if (!last.empty())
    return to_deep_object(path, folly::dynamic::object(last, unwrapped));
  return to_deep_object(path, unwrapped);
}
void DbManager::deep_merge(folly::dynamic &dest, folly::dynamic &merge_obj) {
  if (merge_obj.isObject()) {

    for(auto &pair: merge_obj.items()) {
      if (pair.second.isObject()) {
        if (dest.find(pair.first) == dest.items().end()) {
          dest[pair.first] = pair.second;
        } else {
          deep_merge(dest[pair.first], pair.second);
        }
      } else {
        dest[pair.first] = pair.second;
      }
    }
  }
}
void DbManager::post(const std::string &path, const folly::dynamic &data) {
  LOG(INFO) << "Request to post to " << path;
  if (!path_exists(path)) {
    std::vector<std::string> tokens;

    boost::split(tokens, path, boost::is_any_of("/"));
    LOG(INFO) << "Tokens: " << tokens.size() << " - " <<  tokens;
    folly::dynamic deep_obj = to_deep_object(tokens, data);
    LOG(INFO) << "Deep obj: " << folly::toPrettyJson(deep_obj);
    deep_merge(_root, deep_obj);
  }
}

std::shared_ptr<DbManager> DbManager::get_instance() {
  return the_instance.try_get();
}
void DbManager::put(const std::string &path, const folly::dynamic &data) {
  LOG(INFO) << "Request to put to: " << path;
  if (path_exists(path)) {
    auto tokens = get_path_parts(path);
    auto aux = &_root;
    for (auto part = tokens.begin(); part != tokens.end(); ++part){
      if (std::distance(part, tokens.end()) == 1) {
        //(*aux)[*part] = data;
        aux->insert(*part, data);
        return;
      }

      aux = &aux->at(*part);

    }
  }
}
void DbManager::remove(const std::string &path) {
  //folly::make_exception_wrapper()
  if (path_exists(path)) {
    auto parts = get_path_parts(path);
    auto aux = & _root;

    for (auto part = parts.begin(); part != parts.end(); ++part ) {
      if (std::distance(part, parts.end()) == 1) {
        //last part
        aux->erase(*part);
        return;
      }
      if (aux->isObject()) {
        aux = &aux->at(*part);
      } else {
        LOG(ERROR) << "uh oh";
      }
    }
  }
}
}
