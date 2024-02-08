#include "statsig_client.h"

#include <utility>
#include "statsig_event.h"

namespace statsig {

StatsigClient &StatsigClient::Shared() {
  static StatsigClient inst;
  return inst;
}

void StatsigClient::Initialize(
    const string &sdk_key,
    const optional<StatsigUser> &user,
    const optional<StatsigOptions> &options
) {
  context_.emplace(sdk_key, user, options);
  SetValuesFromNetwork();
}

void StatsigClient::Shutdown() {
  if (!EnsureInitialized(__func__)) {
    return;
  }

  context_->logger.Shutdown();
}

void StatsigClient::UpdateUser(const StatsigUser &user) {
  if (!EnsureInitialized(__func__)) {
    return;
  }

  context_->user = user;
  SetValuesFromNetwork();
}

void StatsigClient::LogEvent(const StatsigEvent &event) {
  if (!EnsureInitialized(__func__)) {
    return;
  }

  context_->logger.Enqueue(InternalizeEvent(event, context_->user));
}

bool StatsigClient::CheckGate(const std::string &gate_name) {
  if (!EnsureInitialized(__func__)) {
    return false;
  }

  auto gate = GetFeatureGate(gate_name);
  return gate.GetValue();
}

FeatureGate StatsigClient::GetFeatureGate(const string &gate_name) {
  if (!EnsureInitialized(__func__)) {
    return {gate_name, "", "Uninitialized", false};
  }

  auto gate = context_->store.GetGate(gate_name);
  context_->logger.Enqueue(MakeGateExposure(gate_name, context_->user, gate));

  return {gate_name, "", "", UNWRAP(gate.evaluation, value)};
}

DynamicConfig StatsigClient::GetDynamicConfig(const std::string &config_name) {
  if (!EnsureInitialized(__func__)) {
    return {config_name, "", "Uninitialized", std::unordered_map<string, json>()};
  }

  auto config = context_->store.GetConfig(config_name);
  context_->logger.Enqueue(MakeConfigExposure(config_name, context_->user, config));

  return {config_name, "", "", UNWRAP(config.evaluation, value)};
}

Experiment StatsigClient::GetExperiment(const std::string &experiment_name) {
  if (!EnsureInitialized(__func__)) {
    return {experiment_name, "", "Uninitialized", std::unordered_map<string, json>()};
  }

  auto experiment = context_->store.GetConfig(experiment_name);
  context_->logger.Enqueue(MakeConfigExposure(experiment_name, context_->user, experiment));

  return {experiment_name, "", "", UNWRAP(experiment.evaluation, value)};
}

Layer StatsigClient::GetLayer(const std::string &layer_name) {
  if (!EnsureInitialized(__func__)) {
    return {layer_name, "", "", std::unordered_map<string, json>()};
  }

  if (layer_name == "test_layer_with_holdout") {
    auto _ = 1;
  }

  auto logger = &context_->logger;

  auto user = context_->user;
  auto layer = context_->store.GetLayer(layer_name);

  auto log_exposure = [layer_name, layer, user, logger](const std::string &param_name) {
    auto expo = MakeLayerExposure(layer_name, param_name, user, layer);
    expo.stop = layer_name == "test_layer";
    logger->Enqueue(expo);
  };

  return {layer_name, "", "", UNWRAP(layer.evaluation, value), log_exposure};

}

void StatsigClient::SetValuesFromNetwork() {
  auto response = context_->network.FetchValues(context_->user);
  if (response.has_value()) {
    context_->store.SetAndCacheValues(response.value());
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


