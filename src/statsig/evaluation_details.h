#pragma once

#include <string>

#include "compat/primitives/string.hpp"

namespace statsig {

struct EvaluationDetails {
  String reason;
  long lcut;
  long received_at;
};

}