#ifdef STATSIG_TESTS

#include "gtest/gtest.h"
#include "nlohmann/json.hpp"

#include "statsig_compat/network/network_client.hpp"
#include "statsig/statsig.h"
#include "test_helpers.hpp"

using namespace statsig;
using namespace statsig::internal;

class ErrorBoundaryTest : public ::testing::Test {
 protected:
  StatsigOptions options_;
  std::string sdk_key_ = "client-key";
  std::vector<nlohmann::json> requests_;
  std::optional<HttpResponse> response_ = HttpResponse{"{}", 200};

  void SetUp() override {
    requests_ = {};
    NetworkClient::GetInstance()._test_func_ = [&](const HttpRequest request) {
      requests_.emplace_back(nlohmann::json{
          {"api", request.api},
          {"path", request.path},
          {"headers", request.headers},
          {"body", nlohmann::json::parse(request.body)},
      });
      return response_.value();
    };
  }

  StatsigResultCode Run() {
    StatsigClient client;

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

    return result;
  }
};

TEST_F(ErrorBoundaryTest, AsynchronousJsonParseFailure) {
  this->response_.emplace(HttpResponse{"<NOT JSON>", 200});
  auto result = this->Run();

  EXPECT_EQ(result, JsonFailureInitializeResponse);

  EXPECT_EQ(requests_[1]["path"], "/v1/sdk_exception");
  EXPECT_EQ(requests_[1]["body"]["exception"], "JsonFailureInitializeResponse");
}

TEST_F(ErrorBoundaryTest, NoErrorBoundary4xx) {
  this->response_.emplace(HttpResponse{"{}", 400});
  auto result = this->Run();

  EXPECT_EQ(result, NetworkFailureBadStatusCode);

  EXPECT_EQ(requests_.size(), 1);
}

TEST_F(ErrorBoundaryTest, YesErrorBoundary5xx) {
  this->response_.emplace(HttpResponse{"{}", 590});
  auto result = this->Run();

  EXPECT_EQ(result, NetworkFailureBadStatusCode);

  EXPECT_EQ(requests_[1]["path"], "/v1/sdk_exception");
  EXPECT_EQ(requests_[1]["body"]["exception"], "NetworkError_590");
}

#endif