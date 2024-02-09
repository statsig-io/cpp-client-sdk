#pragma once

namespace statsig {

class StableID {
 public:
  string Get() {
    if (stable_id_.has_value()) {
      return stable_id_.value();
    }

    stable_id_ = File::ReadFromCache(kStableIdKey);
    if (stable_id_.has_value()) {
      return stable_id_.value();
    }

    auto id = UUID::v4();
    File::WriteToCache(kStableIdKey, id);
    stable_id_ = id;
    return id;
  }

 private:
  optional<string> stable_id_;
};

}