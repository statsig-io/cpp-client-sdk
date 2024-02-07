#pragma once

#include <string>
#include <shared_mutex>

#include "hashing.hpp"

using namespace std;
using namespace statsig::data;
using namespace statsig::hashing;

namespace statsig {

enum class ValueSource {
  Uninitialized,
  Cache,
  Network,
  Bootstrap
};

struct SourceInfo {
  ValueSource source;
  long received_at;
  long lcut;
};

template<typename T>
struct DetailedEvaluation {
  T evaluation;
  SourceInfo source_info;

  DetailedEvaluation(T e, SourceInfo si)
      : evaluation(e), source_info(si) {}
};

class EvaluationStore {
 public:
  void SetAndCacheValues(InitializeResponse values) {
    unique_lock<shared_mutex> lock(rw_lock_);
    source_info_.source = ValueSource::Network;
    values_ = std::move(values);
  }

  optional<DetailedEvaluation<GateEvaluation>> GetGate(const std::string &gate_name) {
    auto hash = DJB2(gate_name);
    shared_lock<shared_mutex> lock(rw_lock_);

    if (!MapContains(values_.feature_gates, hash)) {
      return nullopt;
    }

    return DetailedEvaluation(values_.feature_gates[hash], source_info_);
  }

  optional<DetailedEvaluation<ConfigEvaluation>> GetConfig(const std::string &config_name) {
    auto hash = DJB2(config_name);
    shared_lock<shared_mutex> lock(rw_lock_);

    if (!MapContains(values_.dynamic_configs, hash)) {
      return nullopt;
    }

    return DetailedEvaluation(values_.dynamic_configs[hash], source_info_);
  }

  optional<DetailedEvaluation<LayerEvaluation>> GetLayer(const std::string &layer_name) {
    auto hash = DJB2(layer_name);
    shared_lock<shared_mutex> lock(rw_lock_);

    if (!MapContains(values_.layer_configs, hash)) {
      return nullopt;
    }

    return DetailedEvaluation(values_.layer_configs[hash], source_info_);
  }
 private:
  std::shared_mutex rw_lock_;
  InitializeResponse values_;
  SourceInfo source_info_;

  template<typename Key, typename Value>
  bool MapContains(const std::unordered_map<Key, Value> &map, const Key &key) {
    return map.find(key) != map.end();
  }
};

}