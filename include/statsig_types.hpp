#pragma once

namespace statsig {

typedef std::unordered_map<std::string, nlohmann::json> json_obj;

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
      const std::string &name,
      const std::string &rule_id,
      const std::string &reason,
      const T &value
  )
      : name_(name),
        rule_id_(rule_id),
        reason_(reason),
        value_(value) {}

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

class DynamicConfig : EvaluatedSpec<json_obj> {
 public:
  json_obj GetValue() {
    return value_;
  }

 protected:
  using EvaluatedSpec<json_obj>::EvaluatedSpec;
};

class Experiment : EvaluatedSpec<json_obj> {
 public:
  json_obj GetValue() {
    return value_;
  }

 protected:
  using EvaluatedSpec<json_obj>::EvaluatedSpec;
};

class Layer : EvaluatedSpec<json_obj> {
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
      const json_obj &value,
      const std::function<void(const std::string &)> &log_param_exposure)
      : EvaluatedSpec<json_obj>(name, rule_id, reason, value),
        log_param_exposure_(log_param_exposure) {}

  using EvaluatedSpec<json_obj>::EvaluatedSpec;

 private:
  std::function<void(const std::string &)> log_param_exposure_;

};

}


