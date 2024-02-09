#pragma once

#include <string>
#include <optional>

namespace statsig {

struct StatsigOptions {
  std::optional<std::string> api{};
};

}
