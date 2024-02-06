#pragma once

#include <string>

namespace statsig {

class EvaluationStore {
 public:
  bool GetGate(std::string gate_name) {
    return false;
  }
};

}