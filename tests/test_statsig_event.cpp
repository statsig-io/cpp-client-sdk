#include "gtest/gtest.h"
#include "statsig/statsig_event.h"

using namespace statsig;

TEST(StatsigEventTest, ConstructorWithDoubleValue) {
  std::string event_name = "test_event";
  double value = 42.0;
  StatsigEvent event(event_name, value);

  EXPECT_EQ(event.event_name, event_name);
  EXPECT_EQ(event.double_value, value);
  EXPECT_FALSE(event.time.has_value());
  EXPECT_FALSE(event.string_value.has_value());
  EXPECT_FALSE(event.metadata.has_value());
}

TEST(StatsigEventTest, ConstructorWithStringValue) {
  std::string event_name = "test_event";
  std::string value = "value";
  StatsigEvent event(event_name, value);

  EXPECT_EQ(event.event_name, event_name);
  EXPECT_EQ(event.string_value, value);
  EXPECT_FALSE(event.time.has_value());
  EXPECT_FALSE(event.double_value.has_value());
  EXPECT_FALSE(event.metadata.has_value());
}

TEST(StatsigEventTest, ConstructorWithMetadata) {
  std::string event_name = "test_event";
  StrMap metadata = {{"key1", "value1"}, {"key2", "value2"}};
  StatsigEvent event(event_name, metadata);

  EXPECT_EQ(event.event_name, event_name);
  EXPECT_EQ(event.metadata, metadata);
  EXPECT_FALSE(event.time.has_value());
  EXPECT_FALSE(event.double_value.has_value());
  EXPECT_FALSE(event.string_value.has_value());
}

TEST(StatsigEventTest, ConstructorWithDoubleValueAndMetadata) {
  std::string event_name = "test_event";
  double value = 42.0;
  StrMap metadata = {{"key1", "value1"}, {"key2", "value2"}};
  StatsigEvent event(event_name, value, metadata);

  EXPECT_EQ(event.event_name, event_name);
  EXPECT_EQ(event.double_value, value);
  EXPECT_EQ(event.metadata, metadata);
  EXPECT_FALSE(event.time.has_value());
  EXPECT_FALSE(event.string_value.has_value());
}

TEST(StatsigEventTest, ConstructorWithStringValueAndMetadata) {
  std::string event_name = "test_event";
  std::string value = "value";
  StrMap metadata = {{"key1", "value1"}, {"key2", "value2"}};
  StatsigEvent event(event_name, value, metadata);

  EXPECT_EQ(event.event_name, event_name);
  EXPECT_EQ(event.string_value, value);
  EXPECT_EQ(event.metadata, metadata);
  EXPECT_FALSE(event.time.has_value());
  EXPECT_FALSE(event.double_value.has_value());
}

TEST(StatsigEventTest, ConstructorWithoutValueAndMetadata) {
  std::string event_name = "test_event";
  StatsigEvent event(event_name);

  EXPECT_EQ(event.event_name, event_name);
  EXPECT_FALSE(event.time.has_value());
  EXPECT_FALSE(event.double_value.has_value());
  EXPECT_FALSE(event.string_value.has_value());
  EXPECT_FALSE(event.metadata.has_value());
}

