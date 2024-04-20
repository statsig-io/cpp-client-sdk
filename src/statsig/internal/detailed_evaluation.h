#pragma once

#include <optional>
#include "../evaluation_details.h"

namespace statsig::internal {

template<typename T>
struct DetailedEvaluation {
  std::optional<T> evaluation;
  EvaluationDetails details;
};

}
