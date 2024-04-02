#pragma once

#include <string>
#include <shared_mutex>

#include "hashing.hpp"
#include "macros.hpp"
#include "file.hpp"
#include "statsig/evaluations_data_provider.h"
#include "evaluation_details_internal.hpp"

namespace statsig {

using namespace statsig::data;
using namespace statsig::hashing;

template<typename T>
struct DetailedEvaluation {
  std::optional<T> evaluation;
  EvaluationDetails details;
};

class EvaluationStore {
 public:
  void Reset() {
    WRITE_LOCK(rw_lock_);

    values_ = std::nullopt;
    source_info_ = {ValueSource::Loading};
  }

  void Finalize() {
    WRITE_LOCK(rw_lock_);
    if (values_.has_value()) {
      return;
    }

    source_info_ = {ValueSource::NoValues};
  }

  void SetValuesFromData(
      const string &data,
      const ValueSource &source
  ) {
    WRITE_LOCK(rw_lock_);

    values_ = json::parse(data).template get<InitializeResponse>();
    source_info_ = {source, values_->time, Time::now()};
  }

  DetailedEvaluation<GateEvaluation> GetGate(const std::string &gate_name) {
    READ_LOCK(rw_lock_);

    auto hash = DJB2(gate_name);
    if (!values_.has_value() || !MapContains(values_->feature_gates, hash)) {
      return {
          std::nullopt,
          evaluation_details::UnrecognizedFromSourceInfo(source_info_)
      };
    }

    return {
        values_->feature_gates[hash],
        evaluation_details::RecognizedFromSourceInfo(source_info_)
    };
  }

  DetailedEvaluation<ConfigEvaluation> GetConfig(const std::string &config_name) {
    READ_LOCK(rw_lock_);

    auto hash = DJB2(config_name);
    if (!values_.has_value() || !MapContains(values_->dynamic_configs, hash)) {
      return {
          std::nullopt,
          evaluation_details::UnrecognizedFromSourceInfo(source_info_)
      };
    }

    return {
        values_->dynamic_configs[hash],
        evaluation_details::RecognizedFromSourceInfo(source_info_)
    };
  }

  DetailedEvaluation<LayerEvaluation> GetLayer(const std::string &layer_name) {
    READ_LOCK(rw_lock_);

    auto hash = DJB2(layer_name);
    if (!values_.has_value() || !MapContains(values_->layer_configs, hash)) {
      return {
          std::nullopt,
          evaluation_details::UnrecognizedFromSourceInfo(source_info_)
      };
    }

    return {
        values_->layer_configs[hash],
        evaluation_details::RecognizedFromSourceInfo(source_info_)
    };
  }

 private:
  std::shared_mutex rw_lock_;
  std::optional<InitializeResponse> values_;
  evaluation_details::SourceInfo source_info_;

  template<typename Key, typename Value>
  bool MapContains(const std::unordered_map<Key, Value> &map, const Key &key) {
    return map.find(key) != map.end();
  }
};

}