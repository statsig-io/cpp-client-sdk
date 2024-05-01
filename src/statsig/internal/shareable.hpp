#pragma once

#include <mutex>
#include <unordered_map>

#include "macros.hpp"

namespace statsig::internal {

template<class T>
class Shareable {
  using string = std::string;

 public:
  Shareable<T>() = default;

  std::shared_ptr<T> Get(const string &sdk_key) {
    LOCK(mutex_);

    auto it = instances_.find(sdk_key);
    if (it != instances_.end()) {
      return it->second;
    }

    return nullptr;
  }

  void Add(const string &sdk_key, const std::shared_ptr<T> instance) {
    LOCK(mutex_);
    instances_[sdk_key] = instance;
  }

  void Remove(const string &sdk_key) {
    LOCK(mutex_);
    instances_.erase(sdk_key);
  }

 private:
  std::unordered_map<string, std::shared_ptr<T>> instances_;
  std::mutex mutex_;
};

}