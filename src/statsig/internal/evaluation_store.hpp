#pragma once

#include <string>
#include <shared_mutex>

#include "hashing.hpp"
#include "macros.hpp"
#include "../evaluations_data_adapter.h"
#include "evaluation_details_internal.hpp"
#include "unordered_map_util.hpp"

namespace statsig::internal {

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

  EvaluationDetails GetSourceDetails() {
    READ_LOCK(rw_lock_);

    return {
        evaluation_details::GetValueSourceString(source_info_.source),
        source_info_.lcut,
        source_info_.received_at
    };
  }

  StatsigResultCode SetValuesFromDataAdapterResult(
      std::optional<DataAdapterResult> result) {
    if (!result.has_value()) {
      return Ok;
    }

    WRITE_LOCK(rw_lock_);

    auto parsed = Json::Deserialize<data::InitializeResponse>(result->data);
    if (parsed.code != Ok) {
      return parsed.code;
    }

    values_ = parsed.value;
    source_info_.source = result->source;
    source_info_.received_at = result->receivedAt;
    source_info_.lcut = values_->time;

    return Ok;
  }

  DetailedEvaluation<data::GateEvaluation>
  GetGate(const std::string &gate_name) {
    READ_LOCK(rw_lock_);

    auto hash = hashing::DJB2(gate_name);
    if (!values_.has_value() || !MapContains(values_->feature_gates, hash)) {
      return {
          std::nullopt,
          UnrecognizedFromSourceInfo(source_info_)};
    }

    return {
        values_->feature_gates[hash],
        RecognizedFromSourceInfo(source_info_)};
  }

  DetailedEvaluation<data::ConfigEvaluation> GetConfig(
      const std::string &config_name) {
    READ_LOCK(rw_lock_);

    auto hash = hashing::DJB2(config_name);
    if (!values_.has_value() || !MapContains(values_->dynamic_configs, hash)) {
      return {
          std::nullopt,
          UnrecognizedFromSourceInfo(source_info_)};
    }

    return {
        values_->dynamic_configs[hash],
        RecognizedFromSourceInfo(source_info_)};
  }

  DetailedEvaluation<data::LayerEvaluation> GetLayer(
      const std::string &layer_name) {
    READ_LOCK(rw_lock_);

    auto hash = hashing::DJB2(layer_name);
    if (!values_.has_value() || !MapContains(values_->layer_configs, hash)) {
      return {
          std::nullopt,
          UnrecognizedFromSourceInfo(source_info_)};
    }

    return {
        values_->layer_configs[hash],
        RecognizedFromSourceInfo(source_info_)};
  }

 private:
  std::shared_mutex rw_lock_;
  std::optional<data::InitializeResponse> values_;
  evaluation_details::SourceInfo source_info_ = {};
};

}
