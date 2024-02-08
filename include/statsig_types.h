#pragma once

#include <nlohmann/json.hpp>
#include <utility>

using namespace nlohmann;
using namespace std;
using JsonObject = unordered_map<string, json>;

namespace statsig {

template<typename T>
class EvaluatedSpec {
  friend class StatsigClient;

 public:
  string GetName() {
    return name_;
  }

  string GetRuleId() {
    return rule_id_;
  }

  string GetEvaluationReason() {
    return reason_;
  }

 protected:
  EvaluatedSpec(std::string name, std::string rule_id, std::string reason, T value)
      : name_(std::move(name)),
        rule_id_(std::move(rule_id)),
        reason_(std::move(reason)),
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

class DynamicConfig : EvaluatedSpec<JsonObject> {
 public:
  JsonObject GetValue() {
    return value_;
  }

 protected:
  using EvaluatedSpec<JsonObject>::EvaluatedSpec;
};

class Experiment : EvaluatedSpec<JsonObject> {
 public:
  JsonObject GetValue() {
    return value_;
  }

 protected:
  using EvaluatedSpec<JsonObject>::EvaluatedSpec;
};

class Layer : EvaluatedSpec<JsonObject> {
  friend class StatsigClient;

 public:
  json GetValue(const string &parameter_name) {
    log_param_exposure_(parameter_name);
    return value_[parameter_name];
  }

 protected:
  Layer(
      string name,
      string rule_id,
      string reason,
      JsonObject value,
      function<void(const std::string &)> log_param_exposure)
      : EvaluatedSpec<JsonObject>(name, rule_id, reason, value), log_param_exposure_(log_param_exposure) {}

  using EvaluatedSpec<JsonObject>::EvaluatedSpec;

 private:
  function<void(const std::string &)> log_param_exposure_;

};

}


