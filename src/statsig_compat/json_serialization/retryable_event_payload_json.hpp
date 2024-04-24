#pragma once

#include "statsig/internal/statsig_event_internal.hpp"
#include "statsig_event_json.hpp"

namespace statsig::data_types::retryable_event_payload {

inline StatsigResult<nlohmann::json> ToJson(internal::RetryableEventPayload payload) {
  std::vector<nlohmann::json> events_json_arr;
  events_json_arr.reserve(payload.events.size());

  for (const auto &event : payload.events) {
    events_json_arr.push_back(statsig_event::ToJson(event));
  }

  return {
      Ok,
      nlohmann::json{
          {"attempts", payload.attempts},
          {"events", events_json_arr},
      }
  };
}

inline internal::RetryableEventPayload FromJson(const nlohmann::json &json) {
  internal::RetryableEventPayload failure;
  json.at("attempts").get_to(failure.attempts);

  auto events_json_arr = json.at("events").get<std::vector<nlohmann::json>>();

  for (const auto &event_json : events_json_arr) {
    failure.events.push_back(data_types::statsig_event::FromJson(event_json));
  }

  return failure;
}

inline std::vector<internal::RetryableEventPayload> FromJsonArray(const nlohmann::json &json) {
  std::vector<internal::RetryableEventPayload> result;

  auto failures_json = json.get<std::vector<nlohmann::json>>();

  for (const auto &failure_json : failures_json) {
    result.push_back(FromJson(failure_json));
  }

  return result;
}

inline StatsigResult<std::string> Serialize(
    const std::vector<internal::RetryableEventPayload> &failures
) {
  std::vector<nlohmann::json> failures_json;
  failures_json.reserve(failures.size());

  for (const auto &failure : failures) {
    const auto failure_json = ToJson(failure);
    if (!failure_json.value.has_value()) {
      return {UnexpectedError};
    }
    const auto other = nlohmann::json{ failure_json.value.value() }.dump();

    failures_json.push_back(failure_json.value.value());
  }

  const auto result = nlohmann::json(failures_json).dump();

  return {
      Ok,
      result
  };
}

inline StatsigResult<std::vector<internal::RetryableEventPayload>> Deserialize(
    const std::string &input) {

  try {
    auto j = nlohmann::json::parse(input);
    return {Ok, FromJsonArray(j)};
  }
  catch (std::exception &) {
    return {JsonFailureRetryableEventPayload};
  }
}

}
