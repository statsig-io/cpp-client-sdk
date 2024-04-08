#pragma once

#include <unordered_map>
#include <optional>
#include <functional>

#include "evaluation_details.h"
#include "statsig_compatibility/json/json_value.hpp"

namespace statsig {

class BaseSpec {
 public:
  std::string GetName();

  std::string GetRuleId();

  EvaluationDetails GetEvaluationDetails();

  BaseSpec(
      std::string name,
      std::string rule_id,
      EvaluationDetails evaluation_details
  )
      : name_(std::move(name)),
        rule_id_(std::move(rule_id)),
        evaluation_details_(std::move(evaluation_details)) {}

  BaseSpec(
      std::string name,
      EvaluationDetails evaluation_details
  ) : name_(std::move(name)), evaluation_details_(std::move(evaluation_details)) {}

 private:
  std::string name_;
  std::string rule_id_;
  EvaluationDetails evaluation_details_;
};

template<typename T>
class EvaluatedSpec : public BaseSpec {
 public:
  EvaluatedSpec(
      std::string name,
      std::string rule_id,
      EvaluationDetails evaluation_details,
      T value
  )
      : BaseSpec(name, rule_id, evaluation_details),
        value_(std::move(value)) {}

  EvaluatedSpec(
      std::string name,
      EvaluationDetails evaluation_details
  ) : BaseSpec(name, evaluation_details) {}

 protected:
  T value_;
};

class FeatureGate : public EvaluatedSpec<bool> {
 public:
  bool GetValue();

  using EvaluatedSpec<bool>::EvaluatedSpec;
};

class DynamicConfig : public EvaluatedSpec<JsonValue> {
 public:
  JsonValue GetValues();

  using EvaluatedSpec<JsonValue>::EvaluatedSpec;
};

class Experiment : public EvaluatedSpec<JsonValue> {
 public:
  JsonValue GetValues();

  using EvaluatedSpec<JsonValue>::EvaluatedSpec;
};

class Layer : public EvaluatedSpec<JsonValue> {
  friend class StatsigClient;

 public:
  std::optional<JsonValue> GetValue(const std::string &parameter_name);

  Layer(
      const std::string &name,
      const std::string &rule_id,
      const EvaluationDetails &evaluation_details,
      const JsonValue &value,
      const std::function<void(const std::string &)> &log_param_exposure)
      : EvaluatedSpec<JsonValue>(name, rule_id, evaluation_details, value),
        log_param_exposure_(log_param_exposure) {
  }

  using EvaluatedSpec<JsonValue>::EvaluatedSpec;

 private:
  std::function<void(const std::string &)> log_param_exposure_;

};

}


