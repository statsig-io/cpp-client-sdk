#pragma once

#include <mutex>
#include <memory>
#include <utility>
#include <optional>

#include "diagnostic_markers.hpp"
#include "macros.hpp"

namespace statsig::internal {

namespace /* private */ {
using string = std::string;
}

class Diagnostics {
  using string = std::string;

public:
  static std::shared_ptr<Diagnostics> Get(const string& sdk_key) {
    LOCK(static_mutex_);

    auto it = instances_.find(sdk_key);
    if (it != instances_.end()) {
      return it->second;
    }

    std::shared_ptr<Diagnostics> inst(new Diagnostics());
    instances_[sdk_key] = inst;
    return inst;
  }

  static void Shutdown(const string& sdk_key) {
    LOCK(static_mutex_);
    instances_.erase(sdk_key);
  }

  void SetUser(StatsigUser user) {
    user_ = std::move(user);
  }

  void Mark(const markers::Base& marker) {
    LOCK(mutex_);
    if (markers_.size() > constants::kMaxDiagnosticsMarkers) {
      Log::Warn("Diagnostics max reached, unable to add more markers");
      return;
    }
    markers_.push_back(JsonObjectToJsonValue(marker));
  }

  void AppendEvent(std::vector<StatsigEventInternal>& events) {
    LOCK(mutex_);

    if (markers_.empty()) {
      return;
    }

    auto metadata = std::unordered_map<string, JsonValue>{
        {"markers", JsonArrayToJsonValue(markers_)},
        {"context", ToJsonValue("initialize")}
    };

    const auto event = StatsigEventInternal{
        "statsig::diagnostics",
        Time::now(),
        user_,
        std::nullopt,
        std::nullopt,
        metadata
    };

    events.push_back(event);
  }

  Diagnostics(const Diagnostics&) = delete;
  Diagnostics& operator=(const Diagnostics&) = delete;

private:
  static std::unordered_map<string, std::shared_ptr<Diagnostics>> instances_;
  static std::mutex static_mutex_;

  std::vector<JsonValue> markers_;
  std::mutex mutex_;
  StatsigUser user_;

  explicit Diagnostics() {}
};

std::unordered_map<std::string, std::shared_ptr<Diagnostics>>
Diagnostics::instances_;
std::mutex Diagnostics::static_mutex_;

}
