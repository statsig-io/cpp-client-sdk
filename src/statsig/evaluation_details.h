#pragma once

#include <string>

#include "statsig_compat/primitives/string.hpp"

namespace statsig {

struct EvaluationDetails {
  String reason;
  time_t lcut;
  time_t received_at;
};

}