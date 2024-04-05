#pragma once

#include <string>

namespace statsig::data {

typedef std::unordered_map<std::string, std::string> SecondaryExposure;

template<typename T>
struct Evaluation {
  std::string name;
  std::string rule_id;
  T value{};
  std::vector<SecondaryExposure> secondary_exposures;
};

struct GateEvaluation : Evaluation<bool> {
  std::optional<std::string> id_type;
};

struct ConfigEvaluation : Evaluation<ValueMap> {
  std::string id_type;
  std::optional<std::string> group_name;
  bool is_device_based = false;
  bool is_user_in_experiment = false;
  bool is_experiment_active = false;
};

struct LayerEvaluation : ConfigEvaluation {
  std::optional<std::string> allocated_experiment_name;
  std::vector<std::string> explicit_parameters;
  std::vector<SecondaryExposure> undelegated_secondary_exposures;
};

struct InitializeResponse {
  std::optional<std::string> generator;
  long time;
  std::unordered_map<std::string, GateEvaluation> feature_gates;
  std::unordered_map<std::string, ConfigEvaluation> dynamic_configs;
  std::unordered_map<std::string, LayerEvaluation> layer_configs;
};

}
