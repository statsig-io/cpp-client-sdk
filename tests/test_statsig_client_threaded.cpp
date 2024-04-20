#ifdef STATSIG_TESTS

#include "gtest/gtest.h"
#include "mock_eval_data_adapter.hpp"
#include "statsig/statsig.h"
#include "test_helpers.hpp"

using namespace statsig;

TEST(StatsigClientThreadedTest, DoesNotBlock) {
  StatsigClient client;

  auto mock_adapter = new MockEvaluationsDataAdapter();
  mock_adapter->on_get_data_async = [](auto, auto, auto) {
    sleep(5);
  };

  StatsigOptions opts;
  opts.data_adapter = mock_adapter;

  auto completed = RunBlocking(1000, [&client, &opts](auto done) {
    client.InitializeAsync(
        "client-test", [](auto) {}, std::nullopt, opts);
    done();
  });

  EXPECT_TRUE(completed)
      << "Asynchronous task did not complete within the timeout";
}

#endif