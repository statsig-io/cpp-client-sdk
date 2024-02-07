#pragma once

#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace statsig::data {

struct SecondaryExposure {
  string gate;
  string gate_value;
  string rule_id;
};

void to_json(json &j, const SecondaryExposure &se) {
  j = json{{"gate", se.gate}, {"gateValue", se.gate_value}, {"ruleID", se.rule_id}};
}

void from_json(const json &j, SecondaryExposure &se) {
  j.at("gate").get_to(se.gate);
  j.at("gateValue").get_to(se.gate_value);
  j.at("ruleID").get_to(se.rule_id);
}

// end SecondaryExposure

template<typename T>
struct Evaluation {
  string name;
  string rule_id;
  T value;
  vector<SecondaryExposure> secondary_exposures;
};

//template<typename T>
//void to_json(json &j, const Evaluation<T> &e) {
//  j = json{
//      {"name", e.name},
//      {"rule_id", e.rule_id},
//      {"secondary_exposures", e.secondary_exposures},
//  };
//}
//
template<typename T>
void evaluation_from_json(const json &j, Evaluation<T> &e) {
  j.at("name").get_to(e.name);
  j.at("rule_id").get_to(e.rule_id);
  j.at("secondary_exposures").get_to(e.secondary_exposures);
  j.at("value").get_to(e.value);
}

// end Evaluation

struct GateEvaluation : Evaluation<bool> {
  string id_type;
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
  j.at("id_type").get_to(e.id_type);
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

struct LayerEvaluation : ConfigEvaluation {
  string allocated_experiment_name;
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

template<typename T>
void from_json(const json &j, LayerEvaluation &e) {
  evaluation_from_json<Map>(j, e);
  j.at("allocated_experiment_name").get_to(e.allocated_experiment_name);
  j.at("explicit_parameters").get_to(e.explicit_parameters);
  j.at("undelegated_secondary_exposures").get_to(e.undelegated_secondary_exposures);
}

struct InitializeResponse {
  string generator;
  long time;
  unordered_map<string, GateEvaluation> feature_gates;
  unordered_map<string, ConfigEvaluation> dynamic_configs;
  unordered_map<string, LayerEvaluation> layer_configs;

 public:
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(InitializeResponse, generator, time, feature_gates, dynamic_configs, layer_configs);
};

}
