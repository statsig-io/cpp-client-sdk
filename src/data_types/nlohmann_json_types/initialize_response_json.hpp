#pragma once

#include <nlohmann/json.hpp>
#include "../../initialize_response.hpp"

namespace statsig::data_types {

namespace gate_evaluation {

nlohmann::json ToJson(const statsig::data::GateEvaluation &e) {
  return nlohmann::json{
      {"name", e.name},
      {"rule_id", e.rule_id},
      {"secondary_exposures", e.secondary_exposures},
  };
}

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

nlohmann::json ToJson(const statsig::data::ConfigEvaluation &e) {
  return nlohmann::json{
      {"name", e.name},
      {"rule_id", e.rule_id},
      {"secondary_exposures", e.secondary_exposures},
  };
}

statsig::data::ConfigEvaluation FromJson(const nlohmann::json &j) {
  statsig::data::ConfigEvaluation e;
  j.at("name").get_to(e.name);
  j.at("rule_id").get_to(e.rule_id);
  j.at("secondary_exposures").get_to(e.secondary_exposures);
  e.value = ValueMap(j["value"]);
  return e;
}

}

namespace layer_evaluation {

nlohmann::json ToJson(const statsig::data::LayerEvaluation &e) {
  return nlohmann::json{
      {"name", e.name},
      {"rule_id", e.rule_id},
      {"secondary_exposures", e.secondary_exposures},
  };
}

statsig::data::LayerEvaluation FromJson(const nlohmann::json &j) {
  statsig::data::LayerEvaluation e;

  j.at("name").get_to(e.name);
  j.at("rule_id").get_to(e.rule_id);
  j.at("secondary_exposures").get_to(e.secondary_exposures);
  e.value = ValueMap(j["value"]);
  j.at("explicit_parameters").get_to(e.explicit_parameters);
  j.at("undelegated_secondary_exposures").get_to(e.undelegated_secondary_exposures);

  if (j.contains("allocated_experiment_name")) {
    e.allocated_experiment_name = j["allocated_experiment_name"].get<std::string>();
  }

  return e;
}

}

namespace initialize_response {

nlohmann::json ToJson(const statsig::data::InitializeResponse &res) {
  std::unordered_map<std::string, nlohmann::json> gates;
  gates.reserve(res.feature_gates.size());
  for (const auto &entry : res.feature_gates) {
    gates[entry.first] = gate_evaluation::ToJson(entry.second);
  }

  std::unordered_map<std::string, nlohmann::json> configs;
  configs.reserve(res.dynamic_configs.size());
  for (const auto &entry : res.dynamic_configs) {
    configs[entry.first] = config_evaluation::ToJson(entry.second);
  }

  std::unordered_map<std::string, nlohmann::json> layers;
  layers.reserve(res.layer_configs.size());
  for (const auto &entry : res.layer_configs) {
    layers[entry.first] = layer_evaluation::ToJson(entry.second);
  }

  nlohmann::json j{
      {"time", res.time},
      {"feature_gates", gates},
      {"dynamic_configs", configs},
      {"layer_configs", layers},
  };

  if (res.generator.has_value()) {
    j["generator"] = res.generator.value();
  }

  return j;
}

statsig::data::InitializeResponse FromJson(const nlohmann::json &j) {
  statsig::data::InitializeResponse response;

  using JMap = std::unordered_map<std::string, nlohmann::json>;

  j.at("time").get_to(response.time);

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

  if (j.contains("generator")) {
    response.generator = j["generator"].get<std::string>();
  }

  return response;
}

std::string Serialize(const statsig::data::InitializeResponse &response) {
  return ToJson(response).dump();
}

statsig::data::InitializeResponse Deserialize(const std::string &input) {
  auto j = nlohmann::json::parse(input);
  return FromJson(j);
}

}

}