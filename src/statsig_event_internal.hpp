#pragma once

#include <string>
#include "statsig_user_internal.hpp"
#include "initialize_response.hpp"
#include "macros.hpp"
#include "detailed_evaluation.h"
#include "time.hpp"

namespace statsig::internal {

struct StatsigEventInternal {
  using string = std::string;

  std::string event_name;
  long time{};
  StatsigUser user;
  std::optional<string> string_value;
  std::optional<double> double_value;
  std::optional<std::unordered_map<string, string>> metadata;
  std::optional<std::vector<std::unordered_map<string, string>>> secondary_exposures;
};

struct LogEventRequestArgs {
  const std::vector<StatsigEventInternal> &events;
  std::unordered_map<std::string, std::string> statsig_metadata;
};

StatsigEventInternal InternalizeEvent(StatsigEvent event, StatsigUser &user) {
  StatsigEventInternal result;
  result.event_name = event.event_name;
  result.time = event.time.has_value() ? event.time.value() : Time::now();
  result.user = user;
  result.metadata = event.metadata;
  result.double_value = event.double_value;
  result.string_value = event.string_value;
  return result;
}

template<typename T>
StatsigEventInternal MakeExposureEvent(
    const std::string &event_name,
    const StatsigUser &user,
    const std::optional<T> &evaluation,
    const EvaluationDetails &evaluation_details,
    const std::unordered_map<std::string, std::string> &metadata,
    const std::optional<std::vector<data::SecondaryExposure>> &secondary_exposures = std::nullopt
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

  std::unordered_map<std::string, std::string> final_metadata(metadata);
  final_metadata["reason"] = evaluation_details.reason;
  final_metadata["lcut"] = std::to_string(evaluation_details.lcut);
  final_metadata["receivedAt"] = std::to_string(evaluation_details.received_at);

  result.metadata = final_metadata;
  return result;
}

StatsigEventInternal MakeGateExposure(
    const std::string &gate_name,
    const StatsigUser &user,
    const DetailedEvaluation<data::GateEvaluation> &detailed_evaluation
) {
  auto evaluation = detailed_evaluation.evaluation;
  auto value = UNWRAP(evaluation, value);
  auto rule_id = UNWRAP(evaluation, rule_id);

  return MakeExposureEvent(
      "statsig::gate_exposure",
      user,
      evaluation,
      detailed_evaluation.details,
      {
          {"gate", gate_name},
          {"gateValue", value ? "true" : "false"},
          {"ruleID", rule_id}
      }
  );
}

StatsigEventInternal MakeConfigExposure(
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
      {
          {"config", config_name},
          {"ruleID", rule_id},
      }
  );
}

StatsigEventInternal MakeLayerParamExposure(
    const std::string &layer_name,
    const std::string &param_name,
    const StatsigUser &user,
    const DetailedEvaluation<data::LayerEvaluation> &detailed_evaluation
) {
  auto evaluation = detailed_evaluation.evaluation;
  auto rule_id = UNWRAP(evaluation, rule_id);
  auto explicit_params = UNWRAP(evaluation, explicit_parameters);
  auto is_explicit = std::find(explicit_params.begin(), explicit_params.end(), param_name) != explicit_params.end();

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
      {
          {"config", layer_name},
          {"ruleID", rule_id},
          {"parameterName", param_name},
          {"isExplicitParameter", is_explicit ? "true" : "false"},
          {"allocatedExperiment", UNWRAP_OR(allocated, "")}
      },
      exposures
  );
}

}