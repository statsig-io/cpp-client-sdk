#pragma once

#include "statsig/evaluations_data_adapter.h"

namespace statsig::data_types::statsig_user {

std::string Serialize(const StatsigUser &user) {
  return "";
}

StatsigUser Deserialize(const std::string &input) {
  StatsigUser u;
  return u;
}

}