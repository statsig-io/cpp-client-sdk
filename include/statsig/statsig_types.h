#pragma once

#include <unordered_map>
#include <optional>
#include <functional>

#include "evaluation_details.h"
#include "value_map.h"

namespace statsig {

typedef std::any JsonVal;
typedef std::unordered_map<std::string, JsonVal> JsonObj;

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

class DynamicConfig : public EvaluatedSpec<ValueMap> {
 public:
  ValueMap GetValues();

  using EvaluatedSpec<ValueMap>::EvaluatedSpec;
};

class Experiment : public EvaluatedSpec<ValueMap> {
 public:
  ValueMap GetValues();

  using EvaluatedSpec<ValueMap>::EvaluatedSpec;
};

class Layer : public EvaluatedSpec<ValueMap> {
  friend class StatsigClient;

 public:
  std::optional<JsonValue> GetValue(const std::string &parameter_name);

  Layer(
      const std::string &name,
      const std::string &rule_id,
      const EvaluationDetails &evaluation_details,
      const ValueMap &value,
      const std::function<void(const std::string &)> &log_param_exposure)
      : EvaluatedSpec<ValueMap>(name, rule_id, evaluation_details, value),
        log_param_exposure_(log_param_exposure) {
  }

  using EvaluatedSpec<ValueMap>::EvaluatedSpec;

 private:
  std::function<void(const std::string &)> log_param_exposure_;

};

}


