#pragma once

#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace statsig::data {

using SecondaryExposure = unordered_map<string, string>;

template<typename T>
struct Evaluation {
  string name;
  string rule_id;
  T value{};
  vector<SecondaryExposure> secondary_exposures;
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
  optional<string> id_type;
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
    e.id_type = j["id_type"].get<string>();
  }
}

// end GateEvaluation

using Map = unordered_map<string, json>;
struct ConfigEvaluation : Evaluation<Map> {
  string id_type;
  optional<string> group_name;
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
  evaluation_from_json<Map>(j, e);
//  j.at("id_type").get_to(e.id_type);
}

// end ConfigEvaluation

struct LayerEvaluation : ConfigEvaluation {
  optional<string> allocated_experiment_name;
  vector<string> explicit_parameters;
  vector<SecondaryExposure> undelegated_secondary_exposures;
};

void to_json(json &j, const LayerEvaluation &e) {
  j = json{
      {"name", e.name},
      {"rule_id", e.rule_id},
      {"secondary_exposures", e.secondary_exposures},
  };
}

void from_json(const json &j, LayerEvaluation &e) {
  evaluation_from_json<Map>(j, e);
  j.at("explicit_parameters").get_to(e.explicit_parameters);
  j.at("undelegated_secondary_exposures").get_to(e.undelegated_secondary_exposures);

  if (j.contains("allocated_experiment_name")) {
    e.allocated_experiment_name = j["allocated_experiment_name"].get<string>();
  }
}

// end LayerEvaluation

struct InitializeResponse {
  optional<string> generator;
  long time;
  unordered_map<string, GateEvaluation> feature_gates;
  unordered_map<string, ConfigEvaluation> dynamic_configs;
  unordered_map<string, LayerEvaluation> layer_configs;
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
    res.generator = j["generator"].get<string>();
  }
}

// end InitializeResponse

}
