#pragma once

#include <string>

#include "statsig/compat/primitives/string.hpp"

namespace statsig {

struct EvaluationDetails {
  String reason;
  long lcut;
  long received_at;
};

}