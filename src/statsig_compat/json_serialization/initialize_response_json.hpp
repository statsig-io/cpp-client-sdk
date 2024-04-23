#pragma once

#include "nlohmann/json.hpp"
#include "statsig/internal/initialize_response.hpp"
#include "statsig_compat/primitives/json_value.hpp"

namespace statsig::data_types {

namespace gate_evaluation {

statsig::data::GateEvaluation FromJson(const nlohmann::json &j) {
  statsig::data::GateEvaluation e;

  j.at("name").get_to(e.name);
  j.at("rule_id").get_to(e.rule_id);
  j.at("secondary_exposures").get_to(e.secondary_exposures);
  j.at("value").get_to(e.value);

  if (j.contains("id_type")) {
    e.id_type = j["id_type"].get<std::string>();
  }

  return e;
}

}

namespace config_evaluation {

statsig::data::ConfigEvaluation FromJson(const nlohmann::json &j) {
  statsig::data::ConfigEvaluation e;
  j.at("name").get_to(e.name);
  j.at("rule_id").get_to(e.rule_id);
  j.at("secondary_exposures").get_to(e.secondary_exposures);
  e.value = JsonValue(j["value"]);
  return e;
}

}

namespace layer_evaluation {

statsig::data::LayerEvaluation FromJson(const nlohmann::json &j) {
  statsig::data::LayerEvaluation e;

  j.at("name").get_to(e.name);
  j.at("rule_id").get_to(e.rule_id);
  j.at("secondary_exposures").get_to(e.secondary_exposures);
  e.value = JsonValue(j["value"]);
  j.at("explicit_parameters").get_to(e.explicit_parameters);
  j.at("undelegated_secondary_exposures").get_to(e.undelegated_secondary_exposures);

  if (j.contains("allocated_experiment_name")) {
    e.allocated_experiment_name = j["allocated_experiment_name"].get<std::string>();
  }

  return e;
}

}

namespace initialize_response {

using namespace statsig::data;

InitializeResponse FromJson(const nlohmann::json &j) {
  InitializeResponse response;

  using JMap = std::unordered_map<std::string, nlohmann::json>;

  j.at("time").get_to(response.time);
  j.at("has_updates").get_to(response.has_updates);

  auto gates = j["feature_gates"].get<JMap>();
  response.feature_gates.reserve(gates.size());
  for (const auto &entry : gates) {
    response.feature_gates[entry.first] = gate_evaluation::FromJson(entry.second);
  }

  auto configs = j["dynamic_configs"].get<JMap>();
  response.dynamic_configs.reserve(configs.size());
  for (const auto &entry : configs) {
    response.dynamic_configs[entry.first] = config_evaluation::FromJson(entry.second);
  }

  auto layers = j["layer_configs"].get<JMap>();
  response.layer_configs.reserve(layers.size());
  for (const auto &entry : layers) {
    response.layer_configs[entry.first] = layer_evaluation::FromJson(entry.second);
  }

  return response;
}

// Do Not Serialize. Just use the Raw Network string
// std::string Serialize(const statsig::data::InitializeResponse &response) {
//  return ToJson(response).dump();
//}

StatsigResult<InitializeResponse> Deserialize(const std::string &input) {
  try {
    auto j = nlohmann::json::parse(input);
    return {Ok, FromJson(j)};
  }
  catch (std::exception &) {
    return {JsonFailureInitializeResponse};
  }
}

}

}