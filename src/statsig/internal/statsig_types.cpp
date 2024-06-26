#include "../statsig_types.h"

namespace statsig {

String BaseSpec::GetName() {
  return name_;
}

String BaseSpec::GetRuleId() {
  return rule_id_;
}

EvaluationDetails BaseSpec::GetEvaluationDetails() {
  return evaluation_details_;
}

bool FeatureGate::GetValue() {
  return value_;
}

JsonObject DynamicConfig::GetValues() {
  return GetSafeJsonObject(value_);
}

JsonObject Experiment::GetValues() {
  return GetSafeJsonObject(value_);
}

std::optional<String> Experiment::GetGroupName() {
  return group_name_;
}

std::optional<JsonValue> Layer::GetValue(const std::string &parameter_name) {
  log_param_exposure_(parameter_name);
  return GetJsonValueFromJsonObject(ToCompat(parameter_name), value_);
}

std::optional<String> Layer::GetGroupName() {
  return group_name_;
}


}
