#pragma once

#include <shared_mutex>

#include "statsig_compat/async/async_helper.hpp"

#include "statsig_event_internal.hpp"
#include "macros.hpp"
#include "constants.h"

namespace statsig::internal {

class EventLogger {
 public:
  explicit EventLogger(
      std::string sdk_key,
      StatsigOptions &options,
      NetworkService &network)
      : sdk_key_(sdk_key),
        options_(options),
        network_(network),
        is_shutdown_(false),
        max_buffer_size_(options.logging_max_buffer_size.value_or(constants::kMaxQueuedEvents)),
        logging_interval_ms_(options.logging_interval_ms.value_or(constants::kLoggingIntervalMs)){
    RetryFailedEvents();
    StartBackgroundFlusher();
  }

  ~EventLogger() {
    is_shutdown_.store(true);
  }

  void Enqueue(const StatsigEventInternal &event) {
    WRITE_LOCK(rw_lock_);
    events_.push_back(event);

    if (events_.size() > max_buffer_size_) {
      FlushImpl();
    }
  }

  void Shutdown() {
    Flush();
    is_shutdown_.store(true);
  }

  void Flush() {
    WRITE_LOCK(rw_lock_);
    FlushImpl();
  }

 private:
  std::string sdk_key_;
  StatsigOptions &options_;
  NetworkService &network_;
  std::shared_mutex rw_lock_;
  std::vector<StatsigEventInternal> events_;
  std::atomic<bool> is_shutdown_;
  int max_buffer_size_;
  int logging_interval_ms_;

  void SaveFailedEvents(std::vector<StatsigEventInternal> events,
                        const int attempt) {
    std::vector<RetryableEventPayload> failures;
    const auto key = GetFailedEventCacheKey();
    const auto cache = File::ReadFromCache(key);
    if (cache.has_value()) {
      auto parsed = Json::Deserialize<std::vector<RetryableEventPayload>>(
          cache.value());
      failures = parsed.value.value_or(failures);
    }

    failures.insert(failures.begin(), {attempt, events});

    if (failures.size() > constants::kMaxCachedFailedEventPayloadsCount) {
      failures.pop_back();
    }

    auto serialized = Json::Serialize(failures);
    if (serialized.code == Ok && serialized.value.has_value()) {
      File::WriteToCache(key, serialized.value.value());
    }
  }

  void RetryFailedEvents() {
    AsyncHelper::RunInBackground([&] {
      const auto key = GetFailedEventCacheKey();
      const auto cache = File::ReadFromCache(key);
      if (!cache.has_value()) {
        return;
      }

      const auto failures = Json::Deserialize<std::vector<
          RetryableEventPayload>>(
          cache.value());

      File::DeleteFromCache(key);

      if (failures.code != Ok || !failures.value.has_value()) {
        return;
      }

      for (const auto &failure : failures.value.value()) {
        const auto events = failure.events;
        const auto attempt = failure.attempts + 1;
        network_.SendEvents(
            events,
            [&, events, attempt](const NetworkResult<bool> &result) {
              if (!result.value && attempt <=
                  constants::kFailedEventPayloadRetryCount) {
                SaveFailedEvents(events, attempt);
              }
            });
      }
    });
  }

  std::string GetFailedEventCacheKey() {
    return constants::kCachedFailedEventPayloadPrefix + hashing::DJB2(sdk_key_);
  }

  void FlushImpl() {
    if (events_.empty()) {
      return;
    }

    auto local_events = std::move(events_);
    AsyncHelper::RunInBackground([this, local_events]() {
      network_.SendEvents(
          local_events,
          [&, local_events](const NetworkResult<bool> &result) {
            if (!result.value) {
              SaveFailedEvents(local_events, 1);
            }
          });
    });
  }

  void StartBackgroundFlusher() {
    AsyncHelper::RunInBackground([this] {
      while (!is_shutdown_.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(logging_interval_ms_));
        Flush();
      }
    });
  }
};

}
