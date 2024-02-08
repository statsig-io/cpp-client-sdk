#pragma once

namespace statsig {

class StableID {
 public:
  string Get() {
    if (stable_id_.has_value()) {
      return stable_id_.value();
    }

    stable_id_ = File::ReadFromCache("stable_id");
    if (stable_id_.has_value()) {
      return stable_id_.value();
    }

    auto id = UUID::v4();
    File::WriteToCache("stable_id", id);
    stable_id_ = id;
    return id;
  }

 private:
  optional<string> stable_id_;
};

}