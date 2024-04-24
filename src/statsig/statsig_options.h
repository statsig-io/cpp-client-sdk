#pragma once

#include <string>
#include <optional>

#include "evaluations_data_adapter.h"
#include "log_level.h"

namespace statsig {

class EvaluationsDataAdapter;

struct StatsigOptions {
  std::optional<std::string> api{};
  std::optional<EvaluationsDataAdapter *> data_adapter;
  std::optional<LogLevel> output_logger_level;

  std::optional<int> logging_interval_ms;
  std::optional<int> logging_max_buffer_size;
};

}
