#pragma once

#include <string>
#include <unordered_map>

namespace statsig {

typedef std::optional<std::string> opt_string;
typedef std::unordered_map<std::string, std::string> str_map;

struct StatsigUser {
  std::string user_id;
  str_map custom_ids;

  opt_string email;
  opt_string ip;
  opt_string user_agent;
  opt_string country;
  opt_string locale;
  opt_string app_version;
  std::optional<str_map> custom;
  std::optional<str_map> private_attributes;
};

}

