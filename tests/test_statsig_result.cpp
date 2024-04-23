#ifdef STATSIG_TESTS

#include "gtest/gtest.h"

#include "mock_eval_data_adapter.hpp"
#include "statsig/statsig.h"
#include "test_helpers.hpp"

using namespace statsig;

struct SetupArgs {
  StatsigClient *client;
  MockEvaluationsDataAdapter *mock_adapter;
};

static SetupArgs Setup() {
  auto client = new StatsigClient();

  auto mock_adapter = new MockEvaluationsDataAdapter();
  StatsigOptions opts;
  opts.data_adapter = mock_adapter;

  return {client, mock_adapter};
};

class StatsigResultTest : public ::testing::Test {
 protected:
  StatsigClient client_;
  StatsigOptions options_;
  MockEvaluationsDataAdapter *mock_adapter_{};
  std::string sdk_key_ = "client-key";

  void SetUp() override {
    mock_adapter_ = new MockEvaluationsDataAdapter();
    options_.data_adapter = mock_adapter_;
  }
};

TEST_F(StatsigResultTest, SynchronousSuccess) {
  mock_adapter_->on_get_data_sync = [](auto) {
    return StatsigResult<DataAdapterResult>{Ok};
  };
  auto result = client_.InitializeSync(sdk_key_, std::nullopt, options_);
  EXPECT_EQ(result, Ok);
}

TEST_F(StatsigResultTest, SynchronousNetworkFailure) {
  mock_adapter_->on_get_data_sync = [](auto) {
    return StatsigResult<DataAdapterResult>{NetworkFailureBadStatusCode};
  };
  auto result = client_.InitializeSync(sdk_key_, std::nullopt, options_);
  EXPECT_EQ(result, NetworkFailureBadStatusCode);
}

TEST_F(StatsigResultTest, SynchronousInvalidSdkKey) {
  auto result = client_.InitializeSync("");
  EXPECT_EQ(result, InvalidSdkKey);
}

TEST_F(StatsigResultTest, AsynchronousSuccess) {
  mock_adapter_->on_get_data_sync = [](auto) {
    return StatsigResult<DataAdapterResult>{Ok};
  };
  mock_adapter_->on_get_data_async = [](auto, auto, auto cb) {
    cb(StatsigResult<DataAdapterResult>{Ok});
  };

  auto result = InvalidSdkKey;
  RunBlocking(10000, [&](auto done) {
    client_.InitializeAsync(
        sdk_key_,
        [&, done](auto new_result) {
          result = new_result;
          done();
        },
        std::nullopt, options_);
  });

  EXPECT_EQ(result, Ok);
}

TEST_F(StatsigResultTest, AsynchronousNetworkFailure) {
  mock_adapter_->on_get_data_sync = [](auto) {
    return StatsigResult<DataAdapterResult>{Ok};
  };
  mock_adapter_->on_get_data_async = [](auto, auto, auto cb) {
    cb(StatsigResult<DataAdapterResult>{NetworkFailureBadStatusCode});
  };

  auto result = Ok;
  RunBlocking(10000, [&](auto done) {
    client_.InitializeAsync(
        sdk_key_,
        [&, done](auto new_result) {
          result = new_result;
          done();
        },
        std::nullopt, options_);
  });

  EXPECT_EQ(result, NetworkFailureBadStatusCode);
}

#endif