#pragma once

#include <shared_mutex>
#include <utility>

#include "statsig_compat/async/async_helper.hpp"

#include "statsig_event_internal.hpp"
#include "macros.hpp"
#include "constants.h"
#include "statsig_compat/output_logger/log.hpp"

namespace statsig::internal {

class EventLogger {
  using AsyncHelper = statsig_compatibility::AsyncHelper;
  using Log = statsig_compatibility::Log;

 public:
  explicit EventLogger(
      std::string sdk_key,
      StatsigOptions &options,
      NetworkService &network
  )
      : sdk_key_(std::move(sdk_key)),
        options_(options),
        network_(std::make_shared<NetworkService>(network)),
        has_been_shutdown_(false),
        max_buffer_size_(
            options.logging_max_buffer_size.value_or(
                constants::kMaxQueuedEvents)),
        logging_interval_ms_(
            options.logging_interval_ms.value_or(constants::kLoggingIntervalMs)) {
    RetryFailedEvents();
    StartBackgroundFlusher();
  }

  ~EventLogger() {
    has_been_shutdown_.store(true);
  }

  void Enqueue(const StatsigEventInternal &event) {
    WRITE_LOCK(rw_lock_);
    events_.push_back(event);

    Log::Debug("Enqueueing Event - " + event.event_name);

    if (events_.size() > max_buffer_size_) {
      Log::Debug("Max buffer size reached, flushing...");

      FlushImpl(true);
    }
  }

  void Shutdown() {
    Log::Debug("Shutting down EventLogger");
    has_been_shutdown_.store(true);

    WRITE_LOCK(rw_lock_);
    FlushImpl(false);
  }

  void Flush() {
    WRITE_LOCK(rw_lock_);
    FlushImpl(true);
  }

 private:
  std::string sdk_key_;
  StatsigOptions &options_;
  std::shared_ptr<NetworkService> network_;
  std::shared_mutex rw_lock_;
  std::vector<StatsigEventInternal> events_;
  std::atomic<bool> has_been_shutdown_;
  int max_buffer_size_;
  int logging_interval_ms_;
  std::shared_ptr<Diagnostics> diagnostics_ = Diagnostics::Get(sdk_key_);

  void SaveFailedEvents(
      std::vector<StatsigEventInternal> events,
      const int attempt
  ) {
    std::vector<RetryableEventPayload> failures;
    const auto key = GetFailedEventCacheKey();
    const auto cache = File::ReadFromCache(key);
    if (cache.has_value()) {
      auto parsed = Json::Deserialize<std::vector<RetryableEventPayload>>(
          cache.value());
      failures = parsed.value.value_or(failures);
    }

    failures.insert(failures.begin(), {attempt, std::move(events)});

    if (failures.size() > constants::kMaxCachedFailedEventPayloadsCount) {
      failures.pop_back();
    }

    auto serialized = Json::Serialize(failures);
    if (serialized.code == Ok && serialized.value.has_value()) {
      const auto sdk_key = sdk_key_;
      File::WriteToCache(key, serialized.value.value(), [sdk_key](bool success) {
        if (success) {
          return;
        }

        if (const auto eb = ErrorBoundary::Get(sdk_key)) {
          eb->HandleBadResult(kWriteTag, FileFailureRetryableEventPayload, std::nullopt);
        }
      });
    }
  }

  void RetryFailedEvents() {
    std::weak_ptr<NetworkService> weak_net = network_;
    AsyncHelper::RunInBackground([&, weak_net] {
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
        USE_REF(weak_net, shared_net);

        shared_net->SendEvents(
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

  void FlushImpl(bool should_run_async) {
    diagnostics_->AppendEvent(events_);

    if (events_.empty()) {
      return;
    }

    auto local_events = std::move(events_);
    Log::Debug("Flushing Events " + std::to_string(local_events.size()));

    if (!should_run_async) {
      network_->SendEvents(
          local_events,
          [&, local_events](const NetworkResult<bool> &result) {
            if (!result.value) {
              SaveFailedEvents(local_events, 1);
            }
          });
      return;
    }

    std::weak_ptr<NetworkService> weak_net = network_;
    AsyncHelper::RunInBackground([this, weak_net, local_events]() {
      USE_REF(weak_net, shared_net);

      shared_net->SendEvents(
          local_events,
          [&, local_events](const NetworkResult<bool> &result) {
            if (!result.value) {
              SaveFailedEvents(local_events, 1);
            }
          });
    });
  }

  void StartBackgroundFlusher() {
    std::weak_ptr<NetworkService> weak_net = network_;
    AsyncHelper::RunInBackground([this, weak_net] {
      while (!has_been_shutdown_.load()) {
        AsyncHelper::Sleep(logging_interval_ms_);

        if (has_been_shutdown_.load()) {
          break;
        }
        USE_REF(weak_net, shared_net);

        Log::Debug("Attempting background flush...");
        Flush();
      }
    });
  }
};

}
