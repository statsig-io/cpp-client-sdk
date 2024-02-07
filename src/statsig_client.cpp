#include "statsig_client.h"

namespace statsig {

StatsigClient &StatsigClient::Shared() {
  static StatsigClient inst;
  return inst;
}

void StatsigClient::Initialize(
    string sdk_key,
    const optional<StatsigUser> &user,
    const optional<StatsigOptions> &options
) {
  context_ = StatsigContext(sdk_key, user, options);
  SetValuesFromNetwork();
}

void StatsigClient::Shutdown() {

}

void StatsigClient::UpdateUser(StatsigUser *user) {
//  this->context_.user = user;
}

bool StatsigClient::CheckGate(const std::string &gate_name) {
  auto gate = GetFeatureGate(gate_name);
  return gate.GetValue();
}

FeatureGate StatsigClient::GetFeatureGate(const string &gate_name) {
  if (!EnsureInitialized(__func__)) {
    return {gate_name, "", "Uninitialized", false};
  }

  auto gate = context_->store->GetGate(gate_name);

  // log


  return {gate_name, "", "", gate.has_value() && gate->evaluation.value};
//  return FeatureGate::Empty(gate_name);
}

DynamicConfig StatsigClient::GetDynamicConfig(const std::string &config_name) {
  if (!EnsureInitialized(__func__)) {
//    return FeatureGate::Empty(gate_name);
  }

  auto gate = context_->store->GetConfig(config_name);

  // log

  return {"", "", "", std::unordered_map<string, json>()};
}

Experiment StatsigClient::GetExperiment(const std::string &experiment_name) {
  if (!EnsureInitialized(__func__)) {
//    return FeatureGate::Empty(gate_name);
  }

  auto gate = context_->store->GetConfig(experiment_name);

  // log

  return Experiment("", "", "", std::unordered_map<string, json>());
}

Layer StatsigClient::GetLayer(const std::string &layer_name) {
  if (!EnsureInitialized(__func__)) {
//    return FeatureGate::Empty(gate_name);
  }

  auto gate = context_->store->GetLayer(layer_name);

  // log

  return Layer("", "", "", std::unordered_map<string, json>());
}

void StatsigClient::SetValuesFromNetwork() {
  auto response = context_->network->FetchValues(&context_->user);
  if (response.has_value()) {
    context_->store->SetAndCacheValues(response.value());
  }
}

bool StatsigClient::EnsureInitialized(const char *caller) {
  if (context_.has_value()) {
    return true;
  }

  std::cerr << "[Statsig]: Call made to StatsigClient::" << caller << " before StatsigClient::Initialize" << std::endl;
  return false;
}

}


