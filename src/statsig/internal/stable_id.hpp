#pragma once

#include "file.hpp"

namespace statsig:: internal {

class StableID {
  using string = std::string;

 public:
  string Get() {
    if (stable_id_.has_value()) {
      return stable_id_.value();
    }

    stable_id_ = File::ReadFromCache(constants::kStableIdKey);
    if (stable_id_.has_value()) {
      return stable_id_.value();
    }

    auto id = UUID::v4();
    File::WriteToCache(constants::kStableIdKey, id);
    stable_id_ = id;
    return id;
  }

 private:
  std::optional<std::string> stable_id_;
};

}