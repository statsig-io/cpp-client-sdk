#ifdef STATSIG_TESTS

#include "gtest/gtest.h"
#include "nlohmann/json.hpp"

#include "statsig/statsig.h"
#include "test_helpers.hpp"

#include "statsig_compat/network/network_client.hpp"

using namespace statsig;
using namespace statsig::internal;

class ErrorBoundaryTest : public ::testing::Test {
 protected:
  StatsigOptions options_;
  std::string sdk_key_ = "client-key";
  std::vector<nlohmann::json> requests_;
  std::optional<HttpResponse> response_;

  void SetUp() override {
    requests_ = {};
    response_.emplace(HttpResponse{"{}", 200});

    NetworkClient::GetInstance()._test_func_ = [&](const HttpRequest request) {
      if (request.path == "/v1/sdk_exception") {
        requests_.emplace_back(nlohmann::json{
            {"api", request.api},
            {"path", request.path},
            {"headers", request.headers},
            {"body", nlohmann::json::parse(request.body)},
        });
      }

      return response_.value();
    };
  }

  void TearDown() override {
    NetworkClient::Reset();
  }

  StatsigResultCode Run() {
    StatsigClient client;
    options_.output_logger_level = LogLevel::None;

    WipeAllCaching();

    auto result = Ok;
    RunBlocking(1000, [&](auto done) {
      client.InitializeAsync(
          sdk_key_,
          [&, done](auto new_result) {
            result = new_result;
            done();
          },
          std::nullopt, options_);
    });

    RunBlocking(1000, [&](auto done) {
      client.Shutdown(1111);
      done();
    });

    return result;
  }
};

TEST_F(ErrorBoundaryTest, AsynchronousJsonParseFailure) {
  this->response_.emplace(HttpResponse{"<NOT JSON>", 200});
  auto result = this->Run();

  EXPECT_EQ(result, JsonFailureInitializeResponse);

  EXPECT_EQ(requests_[0]["path"], "/v1/sdk_exception");
  EXPECT_EQ(requests_[0]["body"]["exception"], "JsonFailureInitializeResponse");
  EXPECT_EQ(requests_.size(), 1);
}

TEST_F(ErrorBoundaryTest, NoErrorBoundary4xx) {
  this->response_.emplace(HttpResponse{"{}", 400});
  auto result = this->Run();

  EXPECT_EQ(result, NetworkFailureBadStatusCode);

  EXPECT_EQ(requests_.size(), 0);
}

TEST_F(ErrorBoundaryTest, YesErrorBoundary5xx) {
  this->response_.emplace(HttpResponse{"{}", 590});
  auto result = this->Run();

  EXPECT_EQ(result, NetworkFailureBadStatusCode);

  EXPECT_EQ(requests_[0]["path"], "/v1/sdk_exception");
  EXPECT_EQ(requests_[0]["body"]["exception"], "NetworkError_590");
  EXPECT_EQ(requests_.size(), 1);
}

#endif