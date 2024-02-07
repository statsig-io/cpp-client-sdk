#pragma once

#include <string>
#include <utility>
#include <optional>

using namespace std;

namespace statsig {

struct StatsigOptions {
  optional<string> api{};
};

}
