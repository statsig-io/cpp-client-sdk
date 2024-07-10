#pragma once

#include <mutex>
#include <memory>
#include <utility>
#include <optional>
#include <vector>

#include "../statsig_user.h"
#include "constants.h"
#include "diagnostic_markers.hpp"
#include "macros.hpp"
#include "shareable.hpp"
#include "statsig_compat/defines/module_definitions.h"
#include "statsig_compat/output_logger/log.hpp"
#include "statsig_event_internal.hpp"

namespace statsig::internal {

class Diagnostics {
  using string = std::string;
  
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
    LOCK(mutex_);

    user_ = std::move(user);
  }

  void Mark(const markers::Base &marker) {
    LOCK(mutex_);
    if (markers_.size() > constants::kMaxDiagnosticsMarkers) {
      statsig_compatibility::Log::Warn("Diagnostics max reached, unable to add more markers");
      return;
    }
    markers_.push_back(marker.GetData());
  }

  void AppendEvent(std::vector<StatsigEventInternal> &events) {
    LOCK(mutex_);

    if (markers_.empty()) {
      return;
    }

    const auto local_markers = std::move(markers_);

    auto metadata = std::unordered_map<string, JsonValue>{
        {"markers", JsonArrayToJsonValue(local_markers)},
        {"context", StringToJsonValue("initialize")}
    };

    StatsigEventInternal diagostic_event{
        "statsig::diagnostics",
        Time::now(),
        user_,
        std::nullopt,
        std::nullopt,
        metadata
    };

    events.push_back(std::move(diagostic_event));

    statsig_compatibility::Log::Debug("Appended statsig::diagnostics");
  }

  Diagnostics(const Diagnostics &) = delete;
  Diagnostics &operator=(const Diagnostics &) = delete;

 private:
  STATSIG_EXPORT static Shareable<Diagnostics> shareable_;

  std::vector<JsonValue> markers_;
  std::mutex mutex_;
  StatsigUser user_;

  explicit Diagnostics() {}
};

}
