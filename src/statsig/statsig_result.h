#pragma once

#include <optional>
#include <unordered_map>
#include <string>

namespace statsig {

enum StatsigResultCode {
  Ok,
  UnexpectedError,
  InvalidSdkKey,
  JsonFailureDataAdapterResult,
  JsonFailureInitializeRequestArgs,
  JsonFailureInitializeResponse,
  JsonFailureLogEventResponse,
  JsonFailureLogEventRequestArgs,
  JsonFailureRetryableEventPayload,
  JsonFailureStatsigUser,
  JsonFailureNoDeserializerFound,
  JsonFailureNoSerializerFound,
  NetworkFailureBadStatusCode,
  ClientUninitialized,
};

inline StatsigResultCode operator|(StatsigResultCode a, StatsigResultCode b) {
  return static_cast<StatsigResultCode>(static_cast<uint64_t>(a) | static_cast<uint64_t>(b));
}

inline StatsigResultCode operator&(StatsigResultCode a, StatsigResultCode b) {
  return static_cast<StatsigResultCode>(static_cast<uint64_t>(a) & static_cast<uint64_t>(b));
}

template<class T>
struct StatsigResult {
  StatsigResultCode code;
  std::optional<T> value;
  std::optional<std::unordered_map<std::string, std::string>> extra;
};

}
