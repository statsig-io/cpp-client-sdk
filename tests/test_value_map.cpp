#include "gtest/gtest.h"
#include "statsig/statsig_event.h"
#include "statsig/value_map.h"

using namespace statsig;

TEST(ValueMap, Empty) {
  auto map = ValueMap();

  EXPECT_EQ(map.GetValue("foo"), nullptr);
  EXPECT_EQ(map.GetStringValue("bar"), std::nullopt);
}

TEST(ValueMap, WithJson) {
  auto map = ValueMap({{"str", "value"}, {"num", 2}, {"bool", true}});

  EXPECT_EQ(map.GetValue("str"), "value");
  EXPECT_EQ(map.GetStringValue("str"), "value");
  EXPECT_EQ(map.GetStringValue("bool"), std::nullopt);
}
