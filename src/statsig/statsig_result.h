#pragma once

#include <optional>
#include <unordered_map>
#include <string>
#include <cstdint>

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
  FileFailureDataAdapterResult,
  FileFailureStableID,
  FileFailureRetryableEventPayload,
};

inline StatsigResultCode operator|(StatsigResultCode a, StatsigResultCode b) {
  return static_cast<StatsigResultCode>(static_cast<uint64_t>(a) | static_cast<uint64_t>(b));
}

inline StatsigResultCode operator&(StatsigResultCode a, StatsigResultCode b) {
  return static_cast<StatsigResultCode>(static_cast<uint64_t>(a) & static_cast<uint64_t>(b));
}

inline std::string ResultCodeToString(StatsigResultCode code) {
  switch (code) {
    case JsonFailureInitializeRequestArgs:return "JsonFailureInitializeRequestArgs";
    case JsonFailureInitializeResponse:return "JsonFailureInitializeResponse";
    case JsonFailureLogEventResponse:return "JsonFailureLogEventResponse";
    case JsonFailureNoDeserializerFound:return "JsonFailureNoDeserializerFound";
    case JsonFailureStatsigUser:return "JsonFailureStatsigUser";
    case Ok:return "Ok";
    case UnexpectedError:return "UnexpectedError";
    case InvalidSdkKey:return "InvalidSdkKey";
    case JsonFailureDataAdapterResult:return "JsonFailureDataAdapterResult";
    case JsonFailureRetryableEventPayload:return "JsonFailureRetryableEventPayload";
    case NetworkFailureBadStatusCode:return "NetworkFailureBadStatusCode";
    case ClientUninitialized:return "ClientUninitialized";
    case JsonFailureNoSerializerFound:return "JsonFailureNoSerializerFound";
    case JsonFailureLogEventRequestArgs:return "JsonFailureLogEventRequestArgs";
    case FileFailureDataAdapterResult:return "FileFailureDataAdapterResult";
    case FileFailureStableID: return "FileFailureStableID";
    case FileFailureRetryableEventPayload: return "FileFailureRetryableEventPayload";
  }

  return std::to_string(code);
}

template<class T>
struct StatsigResult {
  StatsigResultCode code = Ok;
  std::optional<T> value;
  std::optional<std::unordered_map<std::string, std::string>> extra;
};

}
