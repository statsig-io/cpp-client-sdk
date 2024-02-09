#include <utility>

#pragma once

namespace statsig {

typedef std::unordered_map<std::string, nlohmann::json> JsonObj;

template<typename T>
class EvaluatedSpec {
  friend class StatsigClient;

 public:
  std::string GetName() {
    return name_;
  }

  std::string GetRuleId() {
    return rule_id_;
  }

  std::string GetEvaluationReason() {
    return reason_;
  }

 protected:
  EvaluatedSpec(
      std::string name,
      std::string rule_id,
      std::string reason,
      T value
  )
      : name_(std::move(name)),
        rule_id_(std::move(rule_id)),
        reason_(std::move(reason)),
        value_(std::move(value)) {}

  EvaluatedSpec(
      std::string name,
      std::string reason
  ) : name_(std::move(name)), reason_(std::move(reason)) {}

  T value_;
  std::string name_;
  std::string rule_id_;
  std::string reason_;
};

class FeatureGate : public EvaluatedSpec<bool> {
 public:
  bool GetValue() {
    return value_;
  }

 protected:
  using EvaluatedSpec<bool>::EvaluatedSpec;
};

class DynamicConfig : public EvaluatedSpec<JsonObj> {
 public:
  JsonObj GetValue() {
    return value_;
  }

 protected:
  using EvaluatedSpec<JsonObj>::EvaluatedSpec;
};

class Experiment : public EvaluatedSpec<JsonObj> {
 public:
  JsonObj GetValue() {
    return value_;
  }

 protected:
  using EvaluatedSpec<JsonObj>::EvaluatedSpec;
};

class Layer : public EvaluatedSpec<JsonObj> {
  friend class StatsigClient;

 public:
  nlohmann::json GetValue(const std::string &parameter_name) {
    log_param_exposure_(parameter_name);
    return value_[parameter_name];
  }

 protected:
  Layer(
      const std::string &name,
      const std::string &rule_id,
      const std::string &reason,
      const JsonObj &value,
      const std::function<void(const std::string &)> &log_param_exposure)
      : EvaluatedSpec<JsonObj>(name, rule_id, reason, value),
        log_param_exposure_(log_param_exposure) {}

  using EvaluatedSpec<JsonObj>::EvaluatedSpec;

 private:
  std::function<void(const std::string &)> log_param_exposure_;

};

}


