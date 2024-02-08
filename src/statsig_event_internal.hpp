#pragma once

#include <string>
#include "statsig_user_internal.hpp"
#include "evaluation_store.hpp"
#include "macros.hpp"

namespace statsig {

struct StatsigEventInternal {
  string event_name;
  long time{};
  StatsigUser user;
  optional<string> string_value;
  optional<double> double_value;
  optional<unordered_map<string, string>> metadata;
  optional<vector<unordered_map<string, string>>> secondary_exposures;
  bool stop;
};

void to_json(json &j, const StatsigEventInternal &event) {

  j = json{
      {"eventName", event.event_name},
      {"time", event.time},
      {"user", event.user},
  };

  if (event.metadata.has_value()) {
    j["metadata"] = event.metadata.value();
  }

  if (event.secondary_exposures.has_value()) {
    j["secondaryExposures"] = event.secondary_exposures.value();
  }

  if (event.string_value.has_value()) {
    j["value"] = event.string_value.value();
  } else if (event.double_value.has_value()) {
    j["value"] = event.double_value.value();
  }

  // @nocommit - debug
  if (event.stop) {
    auto a = j.dump();
    auto _ = a;
  }
}

void from_json(const json &j, StatsigEventInternal &event) {
//  j.at("gate").get_to(event.gate);
//  j.at("gateValue").get_to(event.gate_value);
//  j.at("ruleID").get_to(event.rule_id);
}

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
    string event_name,
    const StatsigUser &user,
    optional<T> evaluation,
    const unordered_map<string, string> &metadata,
    const optional<vector<SecondaryExposure>> &secondary_exposures = nullopt
) {
  StatsigEventInternal result;

  result.event_name = std::move(event_name);
  result.time = Time::now();
  result.user = user;

  if (secondary_exposures.has_value()) {
    result.secondary_exposures = secondary_exposures.value();
  } else {
    result.secondary_exposures = UNWRAP(evaluation, secondary_exposures);
  }

  result.metadata = metadata;

  return result;
}

StatsigEventInternal MakeGateExposure(
    const string &gate_name,
    StatsigUser &user,
    DetailedEvaluation<GateEvaluation> &detailed_evaluation
) {
  auto evaluation = detailed_evaluation.evaluation;
  auto value = UNWRAP(evaluation, value);
  auto rule_id = UNWRAP(evaluation, rule_id);

  return MakeExposureEvent(
      "statsig::gate_exposure",
      user,
      evaluation,
      {
          {"gate", gate_name},
          {"gateValue", value ? "true" : "false"},
          {"ruleID", rule_id}
      }
  );
}

StatsigEventInternal MakeConfigExposure(
    const string &config_name,
    StatsigUser &user,
    DetailedEvaluation<ConfigEvaluation> &detailed_evaluation
) {
  auto evaluation = detailed_evaluation.evaluation;
  auto rule_id = UNWRAP(evaluation, rule_id);

  return MakeExposureEvent(
      "statsig::config_exposure",
      user,
      evaluation,
      {
          {"config", config_name},
          {"ruleID", rule_id}
      }
  );
}

StatsigEventInternal MakeLayerExposure(
    const string &layer_name,
    const string &param_name,
    const StatsigUser &user,
    const DetailedEvaluation<LayerEvaluation> &detailed_evaluation
) {
  auto evaluation = detailed_evaluation.evaluation;
  auto rule_id = UNWRAP(evaluation, rule_id);
  auto explicit_params = UNWRAP(evaluation, explicit_parameters);
  auto is_explicit = std::find(explicit_params.begin(), explicit_params.end(), param_name) != explicit_params.end();

  auto exposures = UNWRAP(evaluation, undelegated_secondary_exposures);

  optional<string> allocated;
  if (is_explicit) {
    allocated = UNWRAP(evaluation, allocated_experiment_name);
    exposures = UNWRAP(evaluation, secondary_exposures);
  }

  return MakeExposureEvent(
      "statsig::layer_exposure",
      user,
      evaluation,
      {
          {"config", layer_name},
          {"ruleID", rule_id},
          {"parameterName", param_name},
          {"isExplicitParameter", is_explicit ? "true" : "false"},
          {"allocatedExperiment", UNWRAP_OR(allocated, "")},
      },
      exposures
  );
}

}