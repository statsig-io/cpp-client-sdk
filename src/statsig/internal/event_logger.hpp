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

      FlushImpl();
    }
  }

  void Shutdown() {
    Log::Debug("Shutting down EventLogger");
    has_been_shutdown_.store(true);

    WRITE_LOCK(rw_lock_);
    FlushImpl();
  }

  void Flush() {
    WRITE_LOCK(rw_lock_);
    FlushImpl();
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
  std::shared_ptr<AsyncHelper> async_helper_ = AsyncHelper::Get(sdk_key_);

  void SaveFailedEvents(
      std::vector<StatsigEventInternal> events,
      const int attempt
  ) {
    std::vector<RetryableEventPayload> failures;
    const auto key = GetFailedEventCacheKey();
    const auto cache = File::ReadFromCache(options_, key);
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
      File::WriteToCache(sdk_key_, options_, key, serialized.value.value(), [sdk_key](bool success) {
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
    async_helper_->RunInBackground([&, weak_net] {
      const auto key = GetFailedEventCacheKey();
      const auto cache = File::ReadFromCache(options_, key);
      if (!cache.has_value()) {
        return;
      }

      const auto failures = Json::Deserialize<std::vector<
          RetryableEventPayload>>(
          cache.value());

      File::DeleteFromCache(options_, key);

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

  void FlushImpl() {
    diagnostics_->AppendEvent(events_);

    if (events_.empty()) {
      return;
    }

    auto local_events = std::move(events_);
    Log::Debug("Flushing " + std::to_string(local_events.size()) + " event(s)");

    std::weak_ptr<NetworkService> weak_net = network_;
    async_helper_->RunInBackground([this, weak_net, local_events]() {
      USE_REF(weak_net, shared_net);
      SendEvents(shared_net, local_events);
    });
  }

  void StartBackgroundFlusher() {
    std::weak_ptr<NetworkService> weak_net = network_;

    async_helper_->RunInBackground([this, weak_net] {
      time_t last_attempt = Time::now();

      while (!has_been_shutdown_.load()) {
        if (Time::now() - last_attempt < logging_interval_ms_) {
          AsyncHelper::Sleep(5);
          continue;
        }

        if (has_been_shutdown_.load()) {
          break;
        }
        last_attempt = Time::now();

        USE_REF(weak_net, shared_net);
        Log::Debug("Attempting background flush...");
        Flush();
      }
    });
  }

  void SendEvents(
      const std::shared_ptr<NetworkService> &network,
      const std::vector<StatsigEventInternal> &events
  ) {
    network->SendEvents(
        events,
        [&, events](const NetworkResult<bool> &result) {

          if (!result.value) {
            Log::Debug("Events failed to send " + std::to_string(events.size())
                           + " event(s). Will try again next session.");
            SaveFailedEvents(events, 1);
          } else {
            Log::Debug(std::to_string(events.size()) + " event(s) successfully sent");
          }
        });
  }
};

}
