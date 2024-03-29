#include "gtest/gtest.h"
#include "evaluation_details.h"
#include "statsig_types.hpp"
#include "nlohmann/json.hpp"

using namespace statsig;

TEST(EvaluatedSpecTest, ConstructorWithBoolValue) {
  std::string name = "test_spec";
  std::string rule_id = "rule_1";
  EvaluationDetails evaluation_details{"reason_1", 123, 456};
  bool value = true;

  FeatureGate feature_gate(name, rule_id, evaluation_details, value);

  EXPECT_EQ(feature_gate.GetName(), name);
  EXPECT_EQ(feature_gate.GetRuleId(), rule_id);
  EXPECT_EQ(feature_gate.GetEvaluationDetails().reason, evaluation_details.reason);
  EXPECT_EQ(feature_gate.GetEvaluationDetails().lcut, evaluation_details.lcut);
  EXPECT_EQ(feature_gate.GetEvaluationDetails().received_at, evaluation_details.received_at);
  EXPECT_EQ(feature_gate.GetValue(), value);
}

TEST(EvaluatedSpecTest, ConstructorWithJsonValue) {
  std::string name = "test_spec";
  std::string rule_id = "rule_1";
  EvaluationDetails evaluation_details{"reason_2", 789, 101112};
  JsonObj value = {{"key1", "value1"}, {"key2", "value2"}};

  DynamicConfig dynamic_config(name, rule_id, evaluation_details, value);

  EXPECT_EQ(dynamic_config.GetName(), name);
  EXPECT_EQ(dynamic_config.GetRuleId(), rule_id);
  EXPECT_EQ(dynamic_config.GetEvaluationDetails().reason, evaluation_details.reason);
  EXPECT_EQ(dynamic_config.GetEvaluationDetails().lcut, evaluation_details.lcut);
  EXPECT_EQ(dynamic_config.GetEvaluationDetails().received_at, evaluation_details.received_at);
  EXPECT_EQ(dynamic_config.GetValue(), value);
}

TEST(EvaluatedSpecTest, ConstructorWithJsonObjValue) {
  std::string name = "test_spec";
  std::string rule_id = "rule_1";
  EvaluationDetails evaluation_details{"reason_3", 131415, 161718};
  JsonObj value = {{"key1", "value1"}, {"key2", "value2"}};

  Experiment experiment(name, rule_id, evaluation_details, value);

  EXPECT_EQ(experiment.GetName(), name);
  EXPECT_EQ(experiment.GetRuleId(), rule_id);
  EXPECT_EQ(experiment.GetEvaluationDetails().reason, evaluation_details.reason);
  EXPECT_EQ(experiment.GetEvaluationDetails().lcut, evaluation_details.lcut);
  EXPECT_EQ(experiment.GetEvaluationDetails().received_at, evaluation_details.received_at);
  EXPECT_EQ(experiment.GetValue(), value);
}

TEST(EvaluatedSpecTest, GetValueFromLayer) {
  std::string name = "test_spec";
  std::string rule_id = "rule_1";
  EvaluationDetails evaluation_details{"reason_4", 192021, 222324};
  JsonObj value = {{"key1", "value1"}, {"key2", "value2"}};
  std::string parameter_name = "key1";
  bool param_exposure_logged = false;

  auto log_param_exposure = [&](const std::string &param) {
    if (param == parameter_name) {
      param_exposure_logged = true;
    }
  };

  Layer layer(name, rule_id, evaluation_details, value, log_param_exposure);
  nlohmann::json retrieved_value = layer.GetValue(parameter_name);

  EXPECT_EQ(retrieved_value, value[parameter_name]);
  EXPECT_TRUE(param_exposure_logged);
}
