#pragma once

#include <functional>

#include "internal/shareable.hpp"
#include "statsig_compat/defines/module_definitions.h"

#include "Misc/CoreDelegates.h"
#include "Templates/Function.h"

namespace statsig_compatibility {

class PlatformEventRegistrationHandle final {
public:
  PlatformEventRegistrationHandle() = default;
  ~PlatformEventRegistrationHandle() = default;

  void Reset() {
    // no-op
  }
};

class PlatformEventsHelper {
 public:
  static std::shared_ptr<PlatformEventsHelper> Get(const std::string &sdk_key) {
    auto instance = shareable_.Get(sdk_key);
    if (instance != nullptr) {
      return instance;
    }

    std::shared_ptr<PlatformEventsHelper> new_instance(new PlatformEventsHelper());
    shareable_.Add(sdk_key, new_instance);
    return new_instance;
  }

  PlatformEventRegistrationHandle RegisterOnApplicationWillDeactivateCallback(const std::function<void()> &callback) {
    // no-op
    return PlatformEventRegistrationHandle();
  }

  PlatformEventRegistrationHandle RegisterOnApplicationWillEnterBackgroundCallback(const std::function<void()> &callback) {
    // no-op
    return PlatformEventRegistrationHandle();
  }

 private:
  STATSIG_EXPORT static statsig::internal::Shareable<PlatformEventsHelper> shareable_;
};

}
