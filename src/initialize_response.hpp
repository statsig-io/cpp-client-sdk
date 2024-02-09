#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace statsig::data {

using json = nlohmann::json;
typedef std::unordered_map<std::string, std::string> SecondaryExposure;
typedef std::unordered_map<std::string, json> JsonObj;

template<typename T>
struct Evaluation {
  std::string name;
  std::string rule_id;
  T value{};
  std::vector<SecondaryExposure> secondary_exposures;
};

template<typename T>
void evaluation_from_json(const json &j, Evaluation<T> &e) {
  j.at("name").get_to(e.name);
  j.at("rule_id").get_to(e.rule_id);
  j.at("secondary_exposures").get_to(e.secondary_exposures);
  j.at("value").get_to(e.value);
}

// end Evaluation

struct GateEvaluation : Evaluation<bool> {
  std::optional<std::string> id_type;
};

void to_json(json &j, const GateEvaluation &e) {
  j = json{
      {"name", e.name},
      {"rule_id", e.rule_id},
      {"secondary_exposures", e.secondary_exposures},
  };
}

void from_json(const json &j, GateEvaluation &e) {
  evaluation_from_json<bool>(j, e);

  if (j.contains("id_type")) {
    e.id_type = j["id_type"].get<std::string>();
  }
}

// end GateEvaluation

struct ConfigEvaluation : Evaluation<JsonObj> {
  std::string id_type;
  std::optional<std::string> group_name;
  bool is_device_based = false;
  bool is_user_in_experiment = false;
  bool is_experiment_active = false;
};

void to_json(json &j, const ConfigEvaluation &e) {
  j = json{
      {"name", e.name},
      {"rule_id", e.rule_id},
      {"secondary_exposures", e.secondary_exposures},
  };
}

void from_json(const json &j, ConfigEvaluation &e) {
  evaluation_from_json<JsonObj>(j, e);
//  j.at("id_type").get_to(e.id_type);
}

// end ConfigEvaluation

struct LayerEvaluation : ConfigEvaluation {
  std::optional<std::string> allocated_experiment_name;
  std::vector<std::string> explicit_parameters;
  std::vector<SecondaryExposure> undelegated_secondary_exposures;
};

void to_json(json &j, const LayerEvaluation &e) {
  j = json{
      {"name", e.name},
      {"rule_id", e.rule_id},
      {"secondary_exposures", e.secondary_exposures},
  };
}

void from_json(const json &j, LayerEvaluation &e) {
  evaluation_from_json<JsonObj>(j, e);
  j.at("explicit_parameters").get_to(e.explicit_parameters);
  j.at("undelegated_secondary_exposures").get_to(e.undelegated_secondary_exposures);

  if (j.contains("allocated_experiment_name")) {
    e.allocated_experiment_name = j["allocated_experiment_name"].get<std::string>();
  }
}

// end LayerEvaluation

struct InitializeResponse {
  std::optional<std::string> generator;
  long time;
  std::unordered_map<std::string, GateEvaluation> feature_gates;
  std::unordered_map<std::string, ConfigEvaluation> dynamic_configs;
  std::unordered_map<std::string, LayerEvaluation> layer_configs;
};

void to_json(json &j, const InitializeResponse &res) {
  j = json{
      {"time", res.time},
      {"feature_gates", res.feature_gates},
      {"dynamic_configs", res.dynamic_configs},
      {"layer_configs", res.layer_configs},
  };

  if (res.generator.has_value()) {
    j["generator"] = res.generator.value();
  }
}

void from_json(const json &j, InitializeResponse &res) {
  j.at("time").get_to(res.time);
  j.at("feature_gates").get_to(res.feature_gates);
  j.at("dynamic_configs").get_to(res.dynamic_configs);
  j.at("layer_configs").get_to(res.layer_configs);

  if (j.contains("generator")) {
    res.generator = j["generator"].get<std::string>();
  }
}

// end InitializeResponse

}
