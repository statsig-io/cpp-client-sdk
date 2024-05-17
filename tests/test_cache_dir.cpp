#ifdef STATSIG_TESTS

#include "gtest/gtest.h"
#include "statsig/statsig.h"
#include "test_helpers.hpp"

#include "statsig/internal/constants.h"
#include "statsig_compat/network/network_client.hpp"

const std::string kTestCacheDir = "/tmp/cpp_test/_😱_";
const std::string kActualCacheDirectory = statsig::constants::kCacheDirectory;

using namespace statsig;
using namespace statsig::internal;

class CacheDirTest : public ::testing::Test {
 protected:
  StatsigOptions options_;

  void SetUp() override {
    DeleteDirectory(kTestCacheDir);
    WipeAllCaching();

    NetworkClient::GetInstance()._test_func_ = [&](const auto) {
      return HttpResponse{};
    };
  }

  void TearDown() override {
    NetworkClient::Reset();
  }
};

void SkipForIo() {
  RunBlocking(10, [&](auto done) {
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    done();
  });
}

TEST_F(CacheDirTest, IsCleaned) {
  SkipForIo();

  EXPECT_EQ(ReadFileAbsolute(kTestCacheDir + "/stable_id"), std::nullopt);
  EXPECT_EQ(ReadFileAbsolute(kActualCacheDirectory + "/stable_id"), std::nullopt);
}

TEST_F(CacheDirTest, CanUseCustom) {
  StatsigClient c;
  StatsigUser u;

  StatsigOptions o;
  o.cache_directory = kTestCacheDir;

  c.InitializeSync("client-key", u, o);

  SkipForIo();

  const auto content = ReadFileAbsolute(kTestCacheDir + "/stable_id").value_or("");
  EXPECT_GT(content.size(), 0);
}

TEST_F(CacheDirTest, UsesDefaultDirectory) {
  StatsigClient c;

  c.InitializeSync("client-key");

  SkipForIo();

  const auto content = ReadFileAbsolute(kActualCacheDirectory + "/stable_id").value_or("");
  EXPECT_GT(content.size(), 0);
}


#endif