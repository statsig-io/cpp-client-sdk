#pragma once

#include <string>
#include <optional>

#include "evaluations_data_provider.h"

namespace statsig {

struct StatsigOptions {
  std::optional<std::string> api{};
  std::optional<std::vector<EvaluationsDataProvider *>> providers;
};

}
