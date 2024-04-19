#pragma once

#include <optional>
#include "../public/evaluation_details.h"

namespace statsig::internal {

template<typename T>
struct DetailedEvaluation {
  std::optional<T> evaluation;
  EvaluationDetails details;
};

}
