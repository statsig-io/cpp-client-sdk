#pragma once

#include "statsig/evaluations_data_adapter.h"


namespace statsig::data_types::data_adapter_result {

std::string Serialize( const DataAdapterResult &res) {
  return "";
}

DataAdapterResult Deserialize(const std::string &input) {
  DataAdapterResult res;
  return res;
}

}