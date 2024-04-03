#include "gtest/gtest.h"
#include "statsig/evaluation_details.h"
#include "statsig/statsig_user.h"
#include "../src/statsig_user_internal.hpp"

using namespace statsig;

TEST(StatsigUserTest, EmptyUserCacheKey) {
  StatsigUser user;
  auto key = MakeCacheKey("client-key", user);
  EXPECT_EQ(key, "2542004821");
}


TEST(StatsigUserTest, UserCacheKey) {
  StatsigUser user;
  user.user_id = "a-user";
  user.email = "a@user.com";
  user.custom_ids = {
      {"employeeID", "an-employee"}
  };

  auto key = MakeCacheKey("client-key", user);
  EXPECT_EQ(key, "3027517876");
}
