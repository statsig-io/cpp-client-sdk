#pragma once

#include <string>
#include <unordered_map>

using namespace std;

namespace statsig {

struct StatsigUser {
  string user_id;
  unordered_map<string, string> custom_ids;
};

}

