#ifdef STATSIG_TESTS

#include "gtest/gtest.h"
#include "mock_eval_data_adapter.hpp"
#include "statsig/statsig.h"
#include "test_helpers.hpp"

#include <memory>
#include <vector>
#include <thread>

#include "statsig_compat/network/network_client.hpp"

using namespace statsig;
using namespace statsig::internal;

class StatsigClientThreadedTest : public ::testing::Test {
 protected:
  void SetUp() override {
    NetworkClient::GetInstance()._test_func_ = [&](auto, const auto cb) {
      std::thread([cb](){
        sleep(1);
        cb(HttpResponse{});
      }).detach();
    };
  }

  void TearDown() override {
    NetworkClient::Reset();
  }
};

TEST_F(StatsigClientThreadedTest, DoesNotBlock) {
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

TEST_F(StatsigClientThreadedTest, CanBeCycled) {
  std::string sdk_key = "client-key";

  for (int i = 0; i < 10; ++i) {
    auto client = std::make_shared<StatsigClient>();
    StatsigOptions opts;
    opts.output_logger_level = LogLevel::Debug;

    client->InitializeAsync(sdk_key, [](auto) {}, {}, opts);
    client->LogEvent(StatsigEvent{"my_event"});
    client->Shutdown(100);
    client.reset();
  }

  sleep(1);
}

#endif