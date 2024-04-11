#pragma once

#include "statsig_event_internal.hpp"
#include "macros.hpp"
#include <shared_mutex>

#include "constants.h"
#include "async/async_helper.hpp"

namespace statsig::internal {

class EventLogger {
public:
  explicit EventLogger(
      std::string sdk_key,
      StatsigOptions& options,
      NetworkService& network
      )
    : sdk_key_(sdk_key),
      options_(options),
      network_(network) {
    RetryFailedEvents();
  }

  void Enqueue(const StatsigEventInternal& event) {
    WRITE_LOCK(rw_lock_);
    events_.push_back(event);
  }

  void Shutdown() {
    Flush();
  }

  void Flush() {
    WRITE_LOCK(rw_lock_);

    if (events_.empty()) {
      return;
    }

    auto local_events = std::move(events_);
    network_.SendEvents(local_events, [&, local_events](bool is_success) {
      if (!is_success) {
        SaveFailedEvents(local_events, 1);
      }
    });
  }

private:
  std::string sdk_key_;
  StatsigOptions& options_;
  NetworkService& network_;
  std::shared_mutex rw_lock_;
  std::vector<StatsigEventInternal> events_;

  void SaveFailedEvents(std::vector<StatsigEventInternal> events, const int attempt) {
    std::vector<FailedEventPayload> failures;
    const auto key = GetFailedEventCacheKey();
    const auto cache = File::ReadFromCache(key);
    if (cache.has_value()) {
      failures = Json::Deserialize<std::vector<FailedEventPayload>>(
              cache.value())
          .value_or(failures);
    }

    failures.insert(failures.begin(), {attempt, events});

    if (failures.size() > constants::kMaxCachedFailedEventPayloadsCount) {
      failures.pop_back();
    }

    File::WriteToCache(key, Json::Serialize(failures));
  }

  void RetryFailedEvents() {
    AsyncHelper::RunInBackground([&] {
      const auto key = GetFailedEventCacheKey();
      const auto cache = File::ReadFromCache(key);
      if (!cache.has_value()) {
        return;

      }

      const auto failures = Json::Deserialize<std::vector<FailedEventPayload>>(
          cache.value());

      File::DeleteFromCache(key);

      if (!failures.has_value()) {
        return;
      }

      for (auto failure : failures.value()) {
        const auto events = failure.events;
        const auto attempt = failure.attempts + 1;
        network_.SendEvents(events, [&, events, attempt](bool is_success) {
          if (!is_success && attempt <= constants::kFailedEventPayloadRetryCount) {
            SaveFailedEvents(events, attempt);
          }
        });
      }
    });
  }

  std::string GetFailedEventCacheKey() {
    return constants::kCachedFailedEventPayloadPrefix + hashing::DJB2(sdk_key_);
  }
};

}
