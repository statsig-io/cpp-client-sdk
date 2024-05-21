#pragma once

#include <optional>
#include <functional>

#include "evaluation_details.h"
#include "statsig_compat/defines/module_definitions.h"
#include "statsig_compat/primitives/json_value.hpp"
#include "statsig_compat/primitives/string.hpp"

namespace statsig {

class STATSIG_EXPORT BaseSpec {
 public:
  String GetName();

  String GetRuleId();

  EvaluationDetails GetEvaluationDetails();

  BaseSpec(
      String name,
      String rule_id,
      EvaluationDetails evaluation_details)
      : name_(std::move(name)),
        rule_id_(std::move(rule_id)),
        evaluation_details_(std::move(evaluation_details)) {}

  BaseSpec(
      String name,
      EvaluationDetails evaluation_details)
      : name_(std::move(name)), evaluation_details_(std::move(evaluation_details)) {}

 private:
  String name_;
  String rule_id_;
  EvaluationDetails evaluation_details_;
};

template<typename T>
class EvaluatedSpec : public BaseSpec {
 public:
  EvaluatedSpec(
      String name,
      String rule_id,
      std::optional<String> group_name,
      EvaluationDetails evaluation_details,
      T value)
      : BaseSpec(name, rule_id, evaluation_details),
        value_(std::move(value)),
        group_name_(group_name) {}

  EvaluatedSpec(
      String name,
      EvaluationDetails evaluation_details) : BaseSpec(name, evaluation_details) {}

 protected:
  T value_;
  std::optional<String> group_name_;
};

class STATSIG_EXPORT FeatureGate : public EvaluatedSpec<bool> {
 public:
  bool GetValue();

  using EvaluatedSpec::EvaluatedSpec;
};

class STATSIG_EXPORT DynamicConfig : public EvaluatedSpec<JsonObject> {
 public:
  JsonObject GetValues();

  using EvaluatedSpec::EvaluatedSpec;
};

class STATSIG_EXPORT Experiment : public EvaluatedSpec<JsonObject> {
 public:
  JsonObject GetValues();

  std::optional<String> GetGroupName();

  using EvaluatedSpec::EvaluatedSpec;
};

class STATSIG_EXPORT Layer : public EvaluatedSpec<JsonObject> {
  friend class StatsigClient;

 public:
  Layer(
      const String &name,
      const String &rule_id,
      const std::optional<String> &group_name,
      const EvaluationDetails &evaluation_details,
      const JsonObject &value,
      const std::function<void(const std::string &)> &log_param_exposure)
      : EvaluatedSpec(name, rule_id, group_name, evaluation_details, value),
        log_param_exposure_(log_param_exposure) {
  }
  using EvaluatedSpec::EvaluatedSpec;

  std::optional<String> GetGroupName();

  std::optional<JsonValue> GetValue(const std::string &parameter_name);

 private:
  std::function<void(const std::string &)> log_param_exposure_;
};

}
