#pragma once

#include <utility>

#include "file.hpp"
#include "error_boundary.hpp"

namespace statsig::internal {

namespace /* private */ {
const char *kWriteTag = "sid:write";
}

class StableID {
  using string = std::string;

 public:
  explicit StableID(string sdk_key) : sdk_key_(std::move(sdk_key)) {}

  string Get() {
    if (stable_id_.has_value()) {
      return stable_id_.value();
    }

    stable_id_ = File::ReadFromCache(constants::kStableIdKey);
    if (stable_id_.has_value()) {
      return stable_id_.value();
    }

    auto key = sdk_key_;
    auto id = UUID::v4();
    File::WriteToCache(constants::kStableIdKey, id, [key](bool success) {
      if (success) {
        return;
      }

      if (const auto eb = ErrorBoundary::Get(key)) {
        eb->HandleBadResult(kWriteTag, FileFailureStableID, std::nullopt);
      }
    });
    stable_id_ = id;
    return id;
  }

 private:
  std::optional<std::string> stable_id_;
  std::string sdk_key_;
};

}