#pragma once

#include <string>
#include "statsig_user_internal.hpp"
#include "initialize_response.hpp"
#include "macros.hpp"
#include "detailed_evaluation.h"
#include "time.hpp"

namespace statsig::internal {

struct StatsigEventInternal {
  std::string event_name;
  time_t time{};
  StatsigUser user;
  std::optional<std::string> string_value;
  std::optional<double> double_value;
  std::optional<std::unordered_map<std::string, JsonValue>> metadata;
  std::optional<std::vector<std::unordered_map<std::string, std::string>>>
      secondary_exposures;
};

struct LogEventRequestArgs {
  const std::vector<StatsigEventInternal> &events;
  std::unordered_map<std::string, std::string> statsig_metadata;
};

struct LogEventResponse {
  bool success;
};

struct RetryableEventPayload {
  int attempts;
  std::vector<StatsigEventInternal> events;
};

inline StatsigEventInternal InternalizeEvent(StatsigEvent event,
                                             const StatsigUser &user) {
  StatsigEventInternal result;
  result.event_name = FromCompat(event.event_name);
  result.time = event.time.has_value() ? event.time.value() : Time::now();
  result.user = user;
  result.double_value = event.double_value;

  if (event.string_value.has_value()) {
    result.string_value = FromCompat(event.string_value.value());
  }

  if (event.metadata.has_value()) {
    std::unordered_map<std::string, JsonValue> meta;
    for (const auto &[fst, snd] : event.metadata.value()) {
      meta[FromCompat(fst)] = CompatStringToJsonValue(snd);
    }
    result.metadata = meta;
  }

  return result;
}

template<typename T>
StatsigEventInternal MakeExposureEvent(
    const std::string &event_name,
    const StatsigUser &user,
    const std::optional<T> &evaluation,
    const EvaluationDetails &evaluation_details,
    const std::unordered_map<std::string, JsonValue> &metadata,
    const std::optional<std::vector<data::SecondaryExposure>> &
    secondary_exposures = std::nullopt
) {
  StatsigEventInternal result;

  result.event_name = event_name;
  result.time = Time::now();
  result.user = user;

  if (secondary_exposures.has_value()) {
    result.secondary_exposures = secondary_exposures.value();
  } else {
    result.secondary_exposures = UNWRAP(evaluation, secondary_exposures);
  }

  std::unordered_map final_metadata(metadata);
  final_metadata["reason"] = CompatStringToJsonValue(evaluation_details.reason);
  final_metadata["lcut"] = JsonValueFromNumber(evaluation_details.lcut);
  final_metadata["receivedAt"] = JsonValueFromNumber(
      evaluation_details.received_at);

  result.metadata = final_metadata;
  return result;
}

inline StatsigEventInternal MakeGateExposure(
    const std::string &gate_name,
    const StatsigUser &user,
    const DetailedEvaluation<data::GateEvaluation> &detailed_evaluation
) {
  auto evaluation = detailed_evaluation.evaluation;
  const auto value = UNWRAP(evaluation, value);
  auto rule_id = UNWRAP(evaluation, rule_id);

  return MakeExposureEvent(
      "statsig::gate_exposure",
      user,
      evaluation,
      detailed_evaluation.details,
      StringMapToJsonValueMap({
                                  {"gate", gate_name},
                                  {"gateValue", value ? "true" : "false"},
                                  {"ruleID", rule_id}
                              })
  );
}

inline StatsigEventInternal MakeConfigExposure(
    const std::string &config_name,
    const StatsigUser &user,
    const DetailedEvaluation<data::ConfigEvaluation> &detailed_evaluation
) {
  auto evaluation = detailed_evaluation.evaluation;
  auto rule_id = UNWRAP(evaluation, rule_id);

  return MakeExposureEvent(
      "statsig::config_exposure",
      user,
      evaluation,
      detailed_evaluation.details,
      StringMapToJsonValueMap({
                                  {"config", config_name},
                                  {"ruleID", rule_id},
                              })
  );
}

inline StatsigEventInternal MakeLayerParamExposure(
    const std::string &layer_name,
    const std::string &param_name,
    const StatsigUser &user,
    const DetailedEvaluation<data::LayerEvaluation> &detailed_evaluation
) {
  auto evaluation = detailed_evaluation.evaluation;
  auto rule_id = UNWRAP(evaluation, rule_id);
  auto explicit_params = UNWRAP(evaluation, explicit_parameters);
  auto is_explicit =
      std::find(explicit_params.begin(), explicit_params.end(), param_name) != explicit_params.end();

  auto exposures = UNWRAP(evaluation, undelegated_secondary_exposures);

  std::optional<std::string> allocated;
  if (is_explicit) {
    allocated = UNWRAP(evaluation, allocated_experiment_name);
    exposures = UNWRAP(evaluation, secondary_exposures);
  }

  return MakeExposureEvent(
      "statsig::layer_exposure",
      user,
      evaluation,
      detailed_evaluation.details,
      StringMapToJsonValueMap({
                                  {"config", layer_name},
                                  {"ruleID", rule_id},
                                  {"parameterName", param_name},
                                  {"isExplicitParameter", is_explicit ? "true" : "false"},
                                  {"allocatedExperiment", UNWRAP_OR(allocated, "")}
                              }),
      exposures
  );
}

}
