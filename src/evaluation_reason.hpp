#pragma once

#include <unordered_map>

#include "evaluation_store.hpp"

namespace statsig {

template<typename T>
std::string GetEvaluationReasonFromEvaluation(DetailedEvaluation<T> detailed_evaluation) {
  static const std::unordered_map<ValueSource, std::string> source_map{
      {ValueSource::Uninitialized, "Uninitialized"},
      {ValueSource::Loading, "Loading"},
      {ValueSource::Cache, "Cache"},
      {ValueSource::Network, "Network"},
      {ValueSource::Bootstrap, "Bootstrap"}
  };

  auto source = detailed_evaluation.source_info.source;
  auto result = source_map.at(source);

  if (source == ValueSource::Uninitialized || source == ValueSource::NoValues) {
    return result;
  }

  auto reason =
      detailed_evaluation.evaluation.has_value()
      ? "Recognized"
      : "Unrecognized";

  return result + ":" + reason;

}

}