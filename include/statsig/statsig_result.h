#pragma once

#include <optional>

namespace statsig {

enum StatsigResultCode: uint64_t {
  Ok = 0,
  ClientUninitialized = 1 << 0,
  NetworkFailureStatusCode = 1 << 1,
  JsonFailureDataAdapterResult = 1 << 2,
  JsonFailureInitializeRequestArgs = 1 << 3,
  JsonFailureInitializeResponse = 1 << 4,
  JsonFailureLogEventResponse = 1 << 5,
  JsonFailureRetryableEventPayload = 1 << 6,
  JsonFailureStatsigUser = 1 << 7,
  JsonFailureNoDeserializerFound = 1 << 8,
};

template <class T>
struct StatsigResult {
  StatsigResultCode code;
  std::optional<T> value;
};

}
