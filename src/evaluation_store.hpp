#pragma once

#include <string>
#include <utility>

#include "hashing.hpp"

using namespace statsig::data;
using namespace statsig::hashing;

namespace statsig {

class EvaluationStore {
 public:
  void SetAndCacheValues(InitializeResponse values) {
    this->values_ = std::move(values);
  }

  bool GetGate(const std::string& gate_name) {
    auto hash = DJB2(gate_name);
    auto gate = this->values_.feature_gates[hash];
    return gate.value;
  }

 private:
  InitializeResponse values_;
};

}