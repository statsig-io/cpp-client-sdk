#include <thread>
#include <memory>
#include <future>
#include <chrono>

#include "gtest/gtest.h"
#include "statsig/statsig.h"

using namespace statsig;

class MockEvaluationsDataProvider : public EvaluationsDataProvider {
 public:
  MockEvaluationsDataProvider() : is_blocked(true), is_terminal(false) {}

  std::optional<std::string> GetEvaluationsData(
      const std::string &sdk_key,
      const StatsigUser &user
  ) override {
    while (is_blocked) {
      sleep(10);
    }
    return std::nullopt;
  }

  void SetEvaluationsData(
      const std::string &sdk_key,
      const StatsigUser &user,
      const std::string &data
  ) override {

  }

  ValueSource GetSource() override {
    return ValueSource::Cache;
  }

  bool IsTerminal() override {
    return is_terminal;
  }

  void Unblock() {
    is_blocked = false;
  }

  bool is_terminal{};
  bool is_blocked{};
};

TEST(StatsigClientThreadedTest, DoesNotBlock) {
  StatsigClient client;

  StatsigOptions opts;
  opts.providers = {
      new MockEvaluationsDataProvider()
  };

  std::promise<void> promise;
  auto future = promise.get_future();

  std::thread([&promise, &client, &opts]() {
    auto _ = client.Initialize("client-test", std::nullopt, opts);
    promise.set_value();
  }).detach();

  auto status = future.wait_for(std::chrono::seconds(2));
  EXPECT_EQ(status, std::future_status::ready) << "Asynchronous task did not complete within the timeout";
}