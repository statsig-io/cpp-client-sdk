#pragma once

#include <optional>
#include <unordered_map>
#include <string>

namespace statsig {

enum StatsigResultCode : uint64_t {
  Ok = 0ULL,
  UnexpectedError = 1ULL << 0,
  InvalidSdkKey = 1ULL << 1,
  JsonFailureDataAdapterResult = 1ULL << 2,
  JsonFailureInitializeRequestArgs = 1ULL << 3,
  JsonFailureInitializeResponse = 1ULL << 4,
  JsonFailureLogEventResponse = 1ULL << 5,
  JsonFailureRetryableEventPayload = 1ULL << 6,
  JsonFailureStatsigUser = 1ULL << 7,
  JsonFailureNoDeserializerFound = 1ULL << 8,
  NetworkFailureBadStatusCode = 1ULL << 9,
  ClientUninitialized = 1ULL << 10,
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
