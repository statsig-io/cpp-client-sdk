#pragma once

#include <optional>
#include <string>
#include <unordered_map>

#include "statsig_compat/primitives/string.hpp"
#include "statsig_compat/primitives/map.hpp"

namespace statsig {

typedef std::optional<String> OptString;

struct StatsigUser {
  OptString user_id;
  StringMap custom_ids;

  OptString email;
  OptString ip;
  OptString user_agent;
  OptString country;
  OptString locale;
  OptString app_version;
  std::optional<StringMap> custom;
  std::optional<StringMap> private_attributes;
};

}
