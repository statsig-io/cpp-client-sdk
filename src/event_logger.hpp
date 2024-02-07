#pragma once

#include <shared_mutex>

#include "statsig_event_internal.hpp"

namespace statsig {

#define WRITE_LOCK(rw_lock_) std::unique_lock<std::shared_mutex> lock(rw_lock_);
#define READ_LOCK(rw_lock_) std::shared_lock<std::shared_mutex> lock(rw_lock_);

class EventLogger {
 public:
  explicit EventLogger(
      StatsigOptions &options, NetworkService &network
  ) : options_(options), network_(network) {}

  void Enqueue(const StatsigEvent &event, const StatsigUser &user) {
    WRITE_LOCK(rw_lock_);
    events_.push_back(InternalizeEvent(event, user));
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
  vector<StatsigEventInternal> events_;
};

}
