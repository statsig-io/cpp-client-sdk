#pragma once

#include <string>
#include <optional>

#include "evaluations_data_adapter.h"

namespace statsig {

class EvaluationsDataAdapter;
class NetworkProvider;
class JsonProvider;

struct StatsigOptions {
  std::optional<std::string> api{};
  EvaluationsDataAdapter *data_adapter;
  NetworkProvider *network_provider;
  JsonProvider *json_provider;
};

}
