#pragma once

#include <unordered_map>

#include "evaluation_store.hpp"

namespace statsig {

template<typename T>
std::string GetEvaluationReasonFromEvaluation(DetailedEvaluation<T> detailed_evaluation) {
  auto source = detailed_evaluation.source_info.source;
  auto result = GetValueSourceString(source);

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