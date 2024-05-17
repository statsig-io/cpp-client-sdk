#pragma once

#include <string>
#include <optional>

#include "statsig_compat/primitives/string.hpp"
#include "statsig_compat/primitives/file_path.hpp"

#include "evaluations_data_adapter.h"
#include "log_level.h"
#include "statsig_environment.h"

namespace statsig {

class EvaluationsDataAdapter;

struct StatsigOptions {
  std::optional<String> api{};
  std::optional<EvaluationsDataAdapter *> data_adapter{};
  std::optional<LogLevel> output_logger_level{};

  std::optional<int> logging_interval_ms{};
  std::optional<int> logging_max_buffer_size{};

  std::optional<StatsigEnvironment> environment{};

  std::optional<FilePath> cache_directory;
};

}
