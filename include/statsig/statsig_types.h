#pragma once

#include <unordered_map>
#include <nlohmann/json.hpp>

#include "evaluation_details.h"

namespace statsig {

typedef std::unordered_map<std::string, nlohmann::json> JsonObj;

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

class DynamicConfig : public EvaluatedSpec<JsonObj> {
 public:
  JsonObj GetValue();

  using EvaluatedSpec<JsonObj>::EvaluatedSpec;
};

class Experiment : public EvaluatedSpec<JsonObj> {
 public:
  JsonObj GetValue();

  using EvaluatedSpec<JsonObj>::EvaluatedSpec;
};

class Layer : public EvaluatedSpec<JsonObj> {
  friend class StatsigClient;

 public:
  nlohmann::json GetValue(const std::string &parameter_name);

  Layer(
      const std::string &name,
      const std::string &rule_id,
      const EvaluationDetails &evaluation_details,
      const JsonObj &value,
      const std::function<void(const std::string &)> &log_param_exposure)
      : EvaluatedSpec<JsonObj>(name, rule_id, evaluation_details, value),
        log_param_exposure_(log_param_exposure) {}

  using EvaluatedSpec<JsonObj>::EvaluatedSpec;

 private:
  std::function<void(const std::string &)> log_param_exposure_;

};

}


