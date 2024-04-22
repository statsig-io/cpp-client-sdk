#pragma once

#include <optional>
#include <functional>

#include "evaluation_details.h"
#include "statsig_compat/primitives/json_value.hpp"
#include "statsig_compat/primitives/string.hpp"

namespace statsig {

class BaseSpec {
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
      EvaluationDetails evaluation_details,
      T value)
      : BaseSpec(name, rule_id, evaluation_details),
        value_(std::move(value)) {}

  EvaluatedSpec(
      String name,
      EvaluationDetails evaluation_details) : BaseSpec(name, evaluation_details) {}

 protected:
  T value_;
};

class FeatureGate : public EvaluatedSpec<bool> {
 public:
  bool GetValue();

  using EvaluatedSpec::EvaluatedSpec;
};

class DynamicConfig : public EvaluatedSpec<JsonObject> {
 public:
  JsonObject GetValues();

  using EvaluatedSpec::EvaluatedSpec;
};

class Experiment : public EvaluatedSpec<JsonObject> {
 public:
  JsonObject GetValues();

  using EvaluatedSpec::EvaluatedSpec;
};

class Layer : public EvaluatedSpec<JsonObject> {
  friend class StatsigClient;

 public:
  Layer(
      const String &name,
      const String &rule_id,
      const EvaluationDetails &evaluation_details,
      const JsonObject &value,
      const std::function<void(const std::string &)> &log_param_exposure)
      : EvaluatedSpec(name, rule_id, evaluation_details, value),
        log_param_exposure_(log_param_exposure) {
  }
  using EvaluatedSpec::EvaluatedSpec;

  std::optional<JsonValue> GetValue(const std::string &parameter_name);

 private:
  std::function<void(const std::string &)> log_param_exposure_;
};

}
