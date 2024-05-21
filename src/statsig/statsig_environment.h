#pragma once

#include <optional>

#include "statsig_compat/primitives/string.hpp"

namespace statsig {

struct StatsigEnvironment {
  std::optional<String> tier{};
};

}