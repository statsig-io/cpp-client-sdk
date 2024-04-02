#include <thread>
#include <memory>
#include <future>
#include <chrono>

#include "gtest/gtest.h"
#include "statsig/statsig.h"

using namespace statsig;

class MockEvaluationsDataAdapter : public EvaluationsDataAdapter {
 public:
 public:
  void Attach(
      const std::string &sdk_key
  ) override {
  }

  std::optional<DataAdapterResult> GetDataSync(
      const StatsigUser &user
  ) override {
    return std::nullopt;
  }

  void GetDataAsync(
      const StatsigUser &user,
      const std::optional<DataAdapterResult> &current,
      const std::function<void(std::optional<DataAdapterResult>)> &callback
  ) override {
    sleep(5);
  }

  void SetData(
      const StatsigUser &user,
      const std::string &data
  ) override {
    // TODO: Support Bootstrap
  }

  void PrefetchData(
      const StatsigUser &user,
      const std::function<void(void)> &callback
  ) override {
    // TODO: Support Prefetch
  }
};

TEST(StatsigClientThreadedTest, DoesNotBlock) {
  StatsigClient client;

  StatsigOptions opts;
  opts.data_adapter = new MockEvaluationsDataAdapter();

  std::promise<void> promise;
  auto future = promise.get_future();

  std::thread([&promise, &client, &opts]() {
    client.InitializeAsync("client-test", []() {}, std::nullopt, opts);
    promise.set_value();
  }).detach();

  auto status = future.wait_for(std::chrono::seconds(1));
  EXPECT_TRUE(status == std::future_status::ready) << "Asynchronous task did not complete within the timeout";
}