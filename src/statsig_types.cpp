#include "statsig/statsig_types.h"
#include "statsig/evaluation_details.h"

#include <unordered_map>

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

ValueMap DynamicConfig::GetValues() {
  return value_;
}

ValueMap Experiment::GetValues() {
  return value_;
}

std::optional<JsonValue> Layer::GetValue(const std::string &parameter_name) {
  log_param_exposure_(parameter_name);
  return value_.GetValue(parameter_name);
}

}
