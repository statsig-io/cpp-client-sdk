#pragma once

#include "statsig_event_internal.hpp"
#include "macros.hpp"
#include <shared_mutex>

namespace statsig::internal {

class EventLogger {
 public:
  explicit EventLogger(
      StatsigOptions &options,
      NetworkService &network
  ) : options_(options), network_(network) {}

  void Enqueue(const StatsigEventInternal &event) {
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
    network_.SendEvents(local_events);
  }

 private:
  StatsigOptions &options_;
  NetworkService &network_;
  std::shared_mutex rw_lock_;
  std::vector<StatsigEventInternal> events_;
};

}
