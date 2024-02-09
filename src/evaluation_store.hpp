#pragma once

#include <string>
#include <shared_mutex>

#include "hashing.hpp"
#include "macros.hpp"
#include "file.hpp"
#include "evaluations_data_provider.h"

namespace statsig {

using namespace statsig::data;
using namespace statsig::hashing;

struct SourceInfo {
  ValueSource source;
  long received_at;
  long lcut;
};

template<typename T>
struct DetailedEvaluation {
  std::optional<T> evaluation;
  SourceInfo source_info;
};

class EvaluationStore {
 public:
  void Reset() {
    WRITE_LOCK(rw_lock_);

    values_ = std::nullopt;
    source_info_.source = ValueSource::Loading;
  }

  void Finalize() {
    WRITE_LOCK(rw_lock_);
    if (values_.has_value()) {
      return;
    }

    source_info_.source = ValueSource::NoValues;
  }

  void SetValuesFromData(
      const string &data,
      const ValueSource &source
  ) {
    WRITE_LOCK(rw_lock_);

    values_ = json::parse(data).template get<InitializeResponse>();
    source_info_.source = source;
  }

//  void LoadCacheValues(const string &cache_key) {
//    WRITE_LOCK(rw_lock_);
//
//    values_ = std::nullopt;
//    source_info_.source = ValueSource::Loading;
//
//    auto cached = File::ReadFromCache(cache_key);
//    if (!cached.has_value()) {
//      return;
//    }
//
//    values_ = json::parse(cached.value()).template get<InitializeResponse>();
//    source_info_.source = ValueSource::Cache;
//  }

//  void SetAndCacheValues(
//      const InitializeResponse &values,
//      const string &raw_values,
//      const ValueSource &source,
//      const string &cache_key
//  ) {
//    WRITE_LOCK(rw_lock_);
//    source_info_.source = source;
//    values_ = values;
//
//    File::WriteToCache(cache_key, raw_values);
//    File::RunCacheEviction();
//  }

  DetailedEvaluation<GateEvaluation> GetGate(const std::string &gate_name) {
    READ_LOCK(rw_lock_);

    auto hash = DJB2(gate_name);
    if (!values_.has_value() || !MapContains(values_->feature_gates, hash)) {
      return {std::nullopt, source_info_};
    }

    return {values_->feature_gates[hash], source_info_};
  }

  DetailedEvaluation<ConfigEvaluation> GetConfig(const std::string &config_name) {
    READ_LOCK(rw_lock_);

    auto hash = DJB2(config_name);
    if (!values_.has_value() || !MapContains(values_->dynamic_configs, hash)) {
      return {std::nullopt, source_info_};
    }

    return {values_->dynamic_configs[hash], source_info_};
  }

  DetailedEvaluation<LayerEvaluation> GetLayer(const std::string &layer_name) {
    READ_LOCK(rw_lock_);

    auto hash = DJB2(layer_name);
    if (!values_.has_value() || !MapContains(values_->layer_configs, hash)) {
      return {std::nullopt, source_info_};
    }

    return {values_->layer_configs[hash], source_info_};
  }

 private:
  std::shared_mutex rw_lock_;
  std::optional<InitializeResponse> values_;
  SourceInfo source_info_;

  template<typename Key, typename Value>
  bool MapContains(const std::unordered_map<Key, Value> &map, const Key &key) {
    return map.find(key) != map.end();
  }
};

}