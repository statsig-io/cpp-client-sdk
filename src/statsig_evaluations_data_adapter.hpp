#pragma once

#include "statsig/evaluations_data_adapter.h"
#include "network_service.hpp"

namespace statsig::internal {

std::optional<DataAdapterResult> ReadFromCacheFile(const std::string &cache_key) {
  auto data = File::ReadFromCache(cache_key);
  if (!data.has_value()) {
    return std::nullopt;
  }

  auto result = json::parse(data.value())
      .template get<DataAdapterResult>();

  result.source = ValueSource::Cache;
  return result;
}

void WriteToCacheFile(const std::string &cache_key, const DataAdapterResult &result) {
  File::WriteToCache(cache_key, json(result).dump());
  File::RunCacheEviction();
}

class StatsigEvaluationsDataAdapter : public statsig::EvaluationsDataAdapter {
 public:
  void Attach(
      const std::string &sdk_key
  ) override {
    sdk_key_ = sdk_key;
    network_ = new statsig::NetworkService((string &) sdk_key);
  }

  std::optional<DataAdapterResult> GetDataSync(
      const StatsigUser &user
  ) override {
    const auto cache_key = GetCacheKey(user);
    auto result = MapGetOrNull(in_memory_cache_, cache_key);

    if (result.has_value()) {
      return result;
    }

    auto cache = ReadFromCacheFile(cache_key);
    if (cache.has_value()) {
      AddToInMemoryCache(cache_key, cache.value());
      return cache;
    }

    return std::nullopt;
  }

  void GetDataAsync(
      const StatsigUser &user,
      const std::optional<DataAdapterResult> &current,
      const std::function<void(std::optional<DataAdapterResult>)> &callback
  ) override {
    const auto cache = current ? current : GetDataSync(user);
    const auto latest = FetchLatest(user, cache);
    const auto cache_key = GetCacheKey(user);

    if (!latest.has_value()) {
      callback(std::nullopt);
      return;
    }

    AddToInMemoryCache(cache_key, latest.value());

    if (latest->source == ValueSource::Network) {
      WriteToCacheFile(cache_key, latest.value());
    }

    callback(latest);
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

 private:
  std::optional<std::string> sdk_key_;
  std::unordered_map<std::string, DataAdapterResult> in_memory_cache_ = {};
  statsig::NetworkService *network_;

  std::string GetCacheKey(const StatsigUser &user) {
    const auto key = MakeCacheKey(GetSdkKey(), user);
    return "statsig.cached.evaluations." + key;
  }

  std::string GetSdkKey() {
    if (!sdk_key_->empty()) {
      return sdk_key_.value();
    }

    std::cerr << "[Statsig]: StatsigEvaluationsDataAdapter is not attached to a Client. " << std::endl;
    return "";
  }

  void AddToInMemoryCache(const std::string &cache_key, const DataAdapterResult &result) {
    if (in_memory_cache_.size() < kMaxCacheEntriesCount) {
      in_memory_cache_[cache_key] = result;
      return;
    }

    auto oldest = in_memory_cache_.begin();
    long long smallestReceivedAt = std::numeric_limits<long long>::max();
    for (auto it = in_memory_cache_.begin(); it != in_memory_cache_.end(); ++it) {
      if (it->second.receivedAt < smallestReceivedAt) {
        oldest = it;
        smallestReceivedAt = it->second.receivedAt;
      }
    }

    if (oldest != in_memory_cache_.end()) {
      in_memory_cache_.erase(oldest);
    }

    in_memory_cache_[cache_key] = result;
  }

  std::optional<DataAdapterResult> FetchLatest(
      const StatsigUser &user,
      const std::optional<DataAdapterResult> current
  ) {
    const auto response = network_->FetchValues(user);
    if (!response.has_value()) {
      return std::nullopt;
    }

    DataAdapterResult result;
    result.data = response->raw;
    result.receivedAt = Time::now();
    result.source = ValueSource::Network;
    return result;
  }
};

}