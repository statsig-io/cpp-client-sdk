#pragma once

#include <mutex>
#include <memory>
#include <utility>
#include <optional>

#include "diagnostic_markers.hpp"
#include "macros.hpp"
#include "shareable.hpp"

namespace statsig::internal {

namespace /* private */ {
using string = std::string;
}

class Diagnostics {
 public:
  static std::shared_ptr<Diagnostics> Get(const string &sdk_key) {
    auto instance = shareable_.Get(sdk_key);
    if (instance != nullptr) {
      return instance;
    }

    std::shared_ptr<Diagnostics> new_instance(new Diagnostics());
    shareable_.Add(sdk_key, new_instance);
    return new_instance;
  }

  static void Shutdown(const string &sdk_key) {
    shareable_.Remove(sdk_key);
  }

  void SetUser(StatsigUser user) {
    user_ = std::move(user);
  }

  void Mark(const markers::Base &marker) {
    LOCK(mutex_);
    if (markers_.size() > constants::kMaxDiagnosticsMarkers) {
      Log::Warn("Diagnostics max reached, unable to add more markers");
      return;
    }
    markers_.push_back(marker.get<JsonValue>());
  }

  void AppendEvent(std::vector<StatsigEventInternal> &events) {
    LOCK(mutex_);

    if (markers_.empty()) {
      return;
    }

    auto local_markers = std::move(markers_);
    auto metadata = std::unordered_map<string, JsonValue>{
        {"markers", local_markers},
        {"context", "initialize"}
    };

    auto event = StatsigEventInternal{
        "statsig::diagnostics",
        Time::now(),
        user_,
        std::nullopt,
        std::nullopt,
        metadata
    };

    events.push_back(event);
  }

  Diagnostics(const Diagnostics &) = delete;
  Diagnostics &operator=(const Diagnostics &) = delete;

 private:
  static Shareable<Diagnostics> shareable_;

  std::vector<JsonValue> markers_;
  std::mutex mutex_;
  StatsigUser user_;

  explicit Diagnostics() {}
};

Shareable<Diagnostics> Diagnostics::shareable_ = Shareable<Diagnostics>();

}