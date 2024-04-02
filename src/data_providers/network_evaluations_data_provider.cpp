#pragma once

#include "statsig/evaluations_data_provider.h"
#include "../network_service.hpp"
#include "statsig/statsig_options.h"

namespace statsig::evaluations_data_providers {

NetworkEvaluationsDataProvider::NetworkEvaluationsDataProvider()
    : network_(new NetworkService((string &) "")) {}

std::optional<std::string> NetworkEvaluationsDataProvider::GetEvaluationsData(
    const std::string &sdk_key,
    const StatsigUser &user
) {
  auto result = network_->FetchValues(user);
  return result.has_value() ? std::optional(result->raw) : std::nullopt;
}

ValueSource NetworkEvaluationsDataProvider::GetSource() {
  return ValueSource::Network;
}

bool NetworkEvaluationsDataProvider::IsTerminal() {
  return true;
}

}
