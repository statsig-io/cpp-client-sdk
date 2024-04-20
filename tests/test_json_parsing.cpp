#ifdef STATSIG_TESTS

#include "gtest/gtest.h"

#include "statsig/internal/json_parser.hpp"
#include "statsig/statsig.h"
#include "test_helpers.hpp"

using namespace statsig;
using namespace statsig::internal;
using namespace statsig::data;

TEST(JsonParsingTest, InitializeResponseInvalidJson) {
  auto result = Json::Deserialize<InitializeResponse>("<Not Json>");
  EXPECT_EQ(result.code, JsonFailureInitializeResponse);
}

TEST(JsonParsingTest, InitializeResponseMissingFields) {
  nlohmann::json data{{"has_updates", true}};

  auto result = Json::Deserialize<InitializeResponse>(data.dump());
  EXPECT_EQ(result.code, JsonFailureInitializeResponse);
}

TEST(JsonParsingTest, InitializeResponseValidJson) {
  auto data = ReadFile("initialize.json");
  auto result = Json::Deserialize<InitializeResponse>(data);
  EXPECT_EQ(result.code, Ok);
  EXPECT_EQ(result.value->dynamic_configs.size(), 4);
}

TEST(JsonParsingTest, LogEventRequestValidJson) {
  std::vector<StatsigEventInternal> events = {{"my_event", 123}};
  auto args = LogEventRequestArgs{events, {{"foo", "bar"}}};
  auto result = Json::Serialize(args);

  EXPECT_EQ(result.value.value(), R"({"events":[{"eventName":"my_event","time":123,"user":null}],"statsigMetadata":{"foo":"bar"}})");
}

TEST(JsonParsingTest, LogEventRequestMissingStatsigMetadata) {
  auto args = LogEventRequestArgs{{}};

  auto result = Json::Serialize(args);
  EXPECT_EQ(result.value, std::nullopt);
  EXPECT_EQ(result.code, JsonFailureLogEventRequestArgs);
}

#endif