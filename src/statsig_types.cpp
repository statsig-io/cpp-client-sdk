#include "statsig/statsig_types.h"

#include <unordered_map>
#include <nlohmann/json.hpp>

#include "statsig/evaluation_details.h"

namespace statsig {

std::string BaseSpec::GetName() {
  return name_;
}

std::string BaseSpec::GetRuleId() {
  return rule_id_;
}

EvaluationDetails BaseSpec::GetEvaluationDetails() {
  return evaluation_details_;
}

bool FeatureGate::GetValue() {
  return value_;
}

JsonObj DynamicConfig::GetValue() {
  return value_;
}

JsonObj Experiment::GetValue() {
  return value_;
}

nlohmann::json Layer::GetValue(const std::string &parameter_name) {
  log_param_exposure_(parameter_name);
  return value_[parameter_name];
}

}


