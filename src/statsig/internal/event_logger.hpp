#pragma once

#include <shared_mutex>
#include <utility>

#include "statsig_compat/async/async_helper.hpp"
#include "statsig_compat/output_logger/log.hpp"
#include "statsig_compat/platform/platform_events_helper.hpp"
#include "statsig_event_internal.hpp"
#include "macros.hpp"
#include "network_service.hpp"
#include "constants.h"

namespace statsig::internal {

class EventLogger {
  using AsyncHelper = statsig_compatibility::AsyncHelper;
  using BackgroundTimerHandle = statsig_compatibility::BackgroundTimerHandle;
  using PlatformEventsHelper = statsig_compatibility::PlatformEventsHelper;
  using PlatformEventRegistrationHandle = statsig_compatibility::PlatformEventRegistrationHandle;
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
        max_buffer_size_(
            options.logging_max_buffer_size.value_or(
                constants::kMaxQueuedEvents)),
        logging_interval_ms_(
            options.logging_interval_ms.value_or(constants::kLoggingIntervalMs)) {
    RetryFailedEvents();
    StartBackgroundFlusher();
    RegisterPlatformEventHandlers();
  }

  ~EventLogger() {
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
    background_flusher_handle_.Reset();
    application_will_deactivate_handle_.Reset();
    application_will_enter_background_handle_.Reset();

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
  int max_buffer_size_;
  int logging_interval_ms_;
  std::shared_ptr<Diagnostics> diagnostics_ = Diagnostics::Get(sdk_key_);
  std::shared_ptr<AsyncHelper> async_helper_ = AsyncHelper::Get(sdk_key_);
  std::shared_ptr<PlatformEventsHelper> platform_events_helper_ = PlatformEventsHelper::Get(sdk_key_);
  BackgroundTimerHandle background_flusher_handle_;
  PlatformEventRegistrationHandle application_will_deactivate_handle_;
  PlatformEventRegistrationHandle application_will_enter_background_handle_;

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
    async_helper_->RunInBackground([this, weak_net] {
      USE_REF(weak_net, shared_net);
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

        shared_net->SendEvents(
            events,
            [this, weak_net, events, attempt](const NetworkResult<bool> &result) {
              USE_REF(weak_net, shared_net);
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
      SendEvents(local_events);
    });
  }

  void StartBackgroundFlusher() {
    std::weak_ptr<NetworkService> weak_net = network_;

    background_flusher_handle_ = async_helper_->StartBackgroundTimer([this, weak_net] {
      USE_REF(weak_net, shared_net);
      Log::Debug("Attempting background flush...");
      Flush();
    }, logging_interval_ms_);
  }

  void RegisterPlatformEventHandlers() {
    std::weak_ptr<NetworkService> weak_net = network_;

    application_will_deactivate_handle_ = platform_events_helper_->RegisterOnApplicationWillDeactivateCallback([this, weak_net] {
      USE_REF(weak_net, shared_net);
      Log::Debug("Flushing due to application deactivation...");
      Flush();
    });

    application_will_enter_background_handle_ = platform_events_helper_->RegisterOnApplicationWillEnterBackgroundCallback([this, weak_net] {
      USE_REF(weak_net, shared_net);
      Log::Debug("Flushing due to application entering background...");
      Flush();
    });
  }

  void SendEvents(
      const std::vector<StatsigEventInternal> &events
  ) {
    std::weak_ptr<NetworkService> weak_net = network_;

    network_->SendEvents(
        events,
        [this, weak_net, events](const NetworkResult<bool> &result) {
          USE_REF(weak_net, shared_net);
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
