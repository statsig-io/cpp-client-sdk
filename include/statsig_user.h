#pragma once

#include <string>
#include <unordered_map>

namespace statsig {

typedef std::optional<std::string> OptString;
typedef std::unordered_map<std::string, std::string> StrMap;

struct StatsigUser {
  std::string user_id;
  StrMap custom_ids;

  OptString email;
  OptString ip;
  OptString user_agent;
  OptString country;
  OptString locale;
  OptString app_version;
  std::optional<StrMap> custom;
  std::optional<StrMap> private_attributes;
};

}

