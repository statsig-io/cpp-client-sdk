#pragma once

#include <string>

namespace statsig {

struct EvaluationDetails {
  std::string reason;
  long lcut;
  long received_at;
};

}