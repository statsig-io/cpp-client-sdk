#pragma once

#include <string>
#include <unordered_map>

using namespace std;

namespace statsig {

struct StatsigUser {
  string user_id;
  unordered_map<string, string> custom_ids;

  optional<string> email;
  optional<string> ip;
  optional<string> user_agent;
  optional<string> country;
  optional<string> locale;
  optional<string> app_version;
  optional<unordered_map<string, string>> custom;
  optional<unordered_map<string, string>> private_attributes;
};

}

