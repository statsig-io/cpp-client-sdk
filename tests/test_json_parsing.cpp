#ifdef STATSIG_TESTS

#include "gtest/gtest.h"

#include "statsig/internal/json_parser.hpp"
#include "statsig/statsig.h"
#include "test_helpers.hpp"

#define EXPECT_CONTAINS(text, substring) \
    EXPECT_PRED2([](const std::string& s, const std::string& sub) { return s.find(sub) != std::string::npos; }, text, substring)

using namespace statsig;
using namespace statsig::internal;
using namespace statsig::data;

/**
 * InitializeResponse
 */

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

/**
 * LogEventRequestArgs
 */

TEST(JsonParsingTest, LogEventRequestValidJson) {
  std::vector<StatsigEventInternal> events = {{"my_event", 123}};
  auto args = LogEventRequestArgs{events, {{"foo", "bar"}}};
  auto result = Json::Serialize(args);

  EXPECT_EQ(result.value.value(),
            R"({"events":[{"eventName":"my_event","time":123,"user":null}],"statsigMetadata":{"foo":"bar"}})");
}

TEST(JsonParsingTest, LogEventRequestMissingStatsigMetadata) {
  auto args = LogEventRequestArgs{{}};

  auto result = Json::Serialize(args);
  EXPECT_EQ(result.value, std::nullopt);
  EXPECT_EQ(result.code, JsonFailureLogEventRequestArgs);
}

/**
 * StatsigUser
 */

std::string valid_statsig_user_json = R"({"customIDs":{"employeeID":"an-employee"},"userID":"a-user"})";

TEST(JsonParsingTest, StatsigUserSerialize) {
  auto user = StatsigUser{"a-user", {{"employeeID", "an-employee"}}};

  auto result = Json::Serialize(user);
  EXPECT_EQ(result.value, valid_statsig_user_json);
}

TEST(JsonParsingTest, StatsigUserParseValidJson) {
  auto result = Json::Deserialize<StatsigUser>(valid_statsig_user_json);

  EXPECT_EQ(result.value->user_id, "a-user");
}

TEST(JsonParsingTest, StatsigUserParseInvalidJson) {
  auto result = Json::Deserialize<StatsigUser>("<Not Json>");

  EXPECT_EQ(result.value, std::nullopt);
  EXPECT_EQ(result.code, JsonFailureStatsigUser);
}

TEST(JsonParsingTest, StatsigUserWithEnvSerialize) {
  StatsigOptions opts;
  opts.environment = {"dev"};
  auto user = internal::NormalizeUser(StatsigUser{"a-user", {{"employeeID", "an-employee"}}}, opts);

  auto result = Json::Serialize(user);
  EXPECT_CONTAINS(result.value.value(), R"("statsigEnvironment":{"tier":"dev"})");
}

TEST(JsonParsingTest, StatsigUserWithEnvDeserialize) {
  const std::string input = R"({"userID":"a-user", "statsigEnvironment":{"tier":"dev"}})";
  auto result = Json::Deserialize<StatsigUser>(input);
  EXPECT_EQ(result.value->statsig_environment->tier, "dev");
}

/**
 * DataAdapterResult
 */

std::string valid_data_adapter_result_json = R"({"data":"{\"foo\":\"bar\"}","fullUserHash":"8888","receivedAt":1234,"source":4})";

TEST(JsonParsingTest, DataAdapterResultSerialize) {
  auto data_adapter = DataAdapterResult{
      statsig::ValueSource::Network,
      R"({"foo":"bar"})",
      1234,
      "8888"
  };

  auto result = Json::Serialize(data_adapter);
  EXPECT_EQ(result.value, valid_data_adapter_result_json);
}

TEST(JsonParsingTest, DataAdapterResultParseValidJson) {
  auto result = Json::Deserialize<DataAdapterResult>(valid_data_adapter_result_json);
  EXPECT_EQ(result.value->data, R"({"foo":"bar"})");
}

TEST(JsonParsingTest, DataAdapterResultInvalidJson) {
  auto result = Json::Deserialize<DataAdapterResult>("<Not Json>");

  EXPECT_EQ(result.value, std::nullopt);
  EXPECT_EQ(result.code, JsonFailureDataAdapterResult);
}

/**
 * RetryableEventPayload
 */

std::string valid_retryable_event_payload_json =
    Trim(R"([{
    "attempts": 1,
    "events": [{
        "eventName": "my_event",
        "metadata": { "key": "value" },
        "time": 123,
        "user": { "userID": "a-user" },
        "value": "a_string"
      }]
  }])");

TEST(JsonParsingTest, RetryableEventPayloadSerialize) {
  std::vector<StatsigEventInternal> events = {
      {
          "my_event",
          123,
          {"a-user"},
          "a_string",
          std::nullopt,
          std::unordered_map<std::string, JsonValue>{{"key", "value"}}}
  };
  auto payload = RetryableEventPayload{1, events};
  auto result = Json::Serialize(std::vector{payload});
  EXPECT_EQ(result.value, valid_retryable_event_payload_json);
}

TEST(JsonParsingTest, RetryableEventPayloadParseValidJson) {
  auto result = Json::Deserialize<std::vector<RetryableEventPayload>>(valid_retryable_event_payload_json);

  EXPECT_EQ(result.code, Ok);
  EXPECT_EQ(result.value.value().size(), 1);

  auto payload = result.value->at(0);
  EXPECT_EQ(payload.attempts, 1);
  EXPECT_EQ(payload.events.size(), 1);

  auto event = payload.events[0];
  EXPECT_EQ(event.event_name, "my_event");
  EXPECT_EQ(event.time, 123);
  EXPECT_EQ(event.user.user_id, "a-user");
}

TEST(JsonParsingTest, RetryableEventPayloadParseInvalidJson) {
  auto result = Json::Deserialize<std::vector<RetryableEventPayload>>("<Not Json>");

  EXPECT_EQ(result.value, std::nullopt);
  EXPECT_EQ(result.code, JsonFailureRetryableEventPayload);
}

#endif