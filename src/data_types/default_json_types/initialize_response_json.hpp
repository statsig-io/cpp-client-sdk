#pragma once

#include "../../initialize_response.hpp"

namespace statsig::data_types::initialize_response {

std::string Serialize(const statsig::data::InitializeResponse &response) {
  return "";
}

statsig::data::InitializeResponse Deserialize(const std::string &input) {
  statsig::data::InitializeResponse response;
  return response;
}

}