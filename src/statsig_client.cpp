#include "statsig_client.h"

#include "statsig_event.hpp"
#include "statsig_context.hpp"

#define INIT_GUARD(result) do { if (!EnsureInitialized(__func__)) { return result; }} while(0)
#define EB(task) context_->err_boundary.Capture(__func__, (task))

namespace statsig {

string GetEvaluationReason(ValueSource source) {
  static const std::unordered_map<ValueSource, std::string> source_map{
      {ValueSource::Uninitialized, "Uninitialized"},
      {ValueSource::Loading, "Loading"},
      {ValueSource::Cache, "Cache"},
      {ValueSource::Network, "Network"},
      {ValueSource::Bootstrap, "Bootstrap"}
  };

  return source_map.at(source);
}

StatsigClient &StatsigClient::Shared() {
  static StatsigClient inst;
  return inst;
}

void StatsigClient::Initialize(
    const string &sdk_key,
    const std::optional<StatsigUser> &user,
    const std::optional<StatsigOptions> &options
) {
  context_ = std::make_unique<StatsigContext>(sdk_key, user, options);
  SwitchUser(context_->user);
}

void StatsigClient::Shutdown() {
  INIT_GUARD();

  EB(([this]() {
    context_->logger.Shutdown();
  }));
}

void StatsigClient::UpdateUser(const StatsigUser &user) {
  INIT_GUARD();
  SwitchUser(user);
}

void StatsigClient::LogEvent(const StatsigEvent &event) {
  INIT_GUARD();

  EB(([this, &event]() {
    context_->logger.Enqueue(InternalizeEvent(event, context_->user));
  }));
}

bool StatsigClient::CheckGate(const std::string &gate_name) {
  INIT_GUARD(false);

  auto gate = GetFeatureGate(gate_name);
  return gate.GetValue();
}

FeatureGate StatsigClient::GetFeatureGate(const string &gate_name) {
  FeatureGate result(gate_name, "Uninitialized");
  INIT_GUARD(result);

  EB(([this, &gate_name, &result]() {
    auto gate = context_->store.GetGate(gate_name);
    context_->logger.Enqueue(MakeGateExposure(gate_name, context_->user, gate));

    result = FeatureGate(
        gate_name,
        UNWRAP(gate.evaluation, rule_id),
        GetEvaluationReason(gate.source_info.source),
        UNWRAP(gate.evaluation, value)
    );
  }));

  return result;
}

DynamicConfig StatsigClient::GetDynamicConfig(const std::string &config_name) {
  DynamicConfig result(config_name, "Uninitialized");
  INIT_GUARD(result);

  EB(([this, &config_name, &result]() {
    auto config = context_->store.GetConfig(config_name);
    context_->logger.Enqueue(MakeConfigExposure(config_name, context_->user, config));

    result = DynamicConfig(
        config_name,
        UNWRAP(config.evaluation, rule_id),
        GetEvaluationReason(config.source_info.source),
        UNWRAP(config.evaluation, value)
    );
  }));

  return result;
}

Experiment StatsigClient::GetExperiment(const std::string &experiment_name) {
  Experiment result(experiment_name, "Uninitialized");
  INIT_GUARD(result);

  EB(([this, &experiment_name, &result]() {
    auto experiment = context_->store.GetConfig(experiment_name);
    context_->logger.Enqueue(MakeConfigExposure(experiment_name, context_->user, experiment));

    result = Experiment(
        experiment_name,
        UNWRAP(experiment.evaluation, rule_id),
        GetEvaluationReason(experiment.source_info.source),
        UNWRAP(experiment.evaluation, value)
    );
  }));

  return result;
}

Layer StatsigClient::GetLayer(const std::string &layer_name) {
  Layer result(layer_name, "Uninitialized");
  INIT_GUARD(result);

  EB(([this, &layer_name, &result]() {
    auto logger = &context_->logger;
    auto user = context_->user;
    auto layer = context_->store.GetLayer(layer_name);

    auto log_exposure = [layer_name, layer, user, logger](const std::string &param_name) {
      auto expo = MakeLayerExposure(layer_name, param_name, user, layer);
      logger->Enqueue(expo);
    };

    result = Layer(
        layer_name,
        UNWRAP(layer.evaluation, rule_id),
        GetEvaluationReason(layer.source_info.source),
        UNWRAP(layer.evaluation, value),
        log_exposure
    );
  }));

  return result;
}

void StatsigClient::SwitchUser(const statsig::StatsigUser &user) {
  EB(([this, &user]() {
    context_->user = user;
    auto cache_key = MakeCacheKey(context_->sdk_key, context_->user);

    context_->store.LoadCacheValues(cache_key);
    SetValuesFromNetwork();
  }));
}

void StatsigClient::SetValuesFromNetwork() {
  auto result = context_->network.FetchValues(context_->user);
  if (result.has_value()) {
    auto cache_key = MakeCacheKey(context_->sdk_key, context_->user);
    context_->store.SetAndCacheValues(result->response, result->raw, ValueSource::Network, cache_key);
  }
}

bool StatsigClient::EnsureInitialized(const char *caller) {
  if (context_) {
    return true;
  }

  std::cerr << "[Statsig]: Call made to StatsigClient::" << caller << " before StatsigClient::Initialize" << std::endl;
  return false;
}

}


