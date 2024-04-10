#include "statsig/statsig_client.h"

#include <utility>

#include "statsig/statsig_event.h"
#include "statsig_context.hpp"
#include "evaluation_details_internal.hpp"
#include "statsig_compatibility/async/async_helper.hpp"

#define INIT_GUARD(result) do { if (!EnsureInitialized(__func__)) { return result; }} while(0)
#define EB_WITH_TAG(tag, task) context_->err_boundary.Capture((tag), (task))
#define EB(task) EB_WITH_TAG(__func__, task)

namespace statsig {

StatsigClient& StatsigClient::Shared() {
  static StatsigClient inst;
  return inst;
}

StatsigClient::StatsigClient() {
  context_.reset();
}

StatsigClient::~StatsigClient() {
  context_.reset();
}

void StatsigClient::InitializeSync(
    const std::string& sdk_key,
    const std::optional<StatsigUser>& user,
    const std::optional<StatsigOptions>& options) {
  EB(([this, &sdk_key, &user, &options]() {
    context_.reset(new StatsigContext(sdk_key, user, options));
    UpdateUserSync(context_->user);
    }));
}

void StatsigClient::InitializeAsync(
    const std::string& sdk_key,
    const std::function<void()>& callback,
    const std::optional<StatsigUser>& user,
    const std::optional<StatsigOptions>& options) {
  EB(([this, &sdk_key, &user, &options, &callback]() {
    context_.reset(new StatsigContext(sdk_key, user, options));
    UpdateUserAsync(context_->user, callback);
    }));
}

void StatsigClient::UpdateUserSync(const StatsigUser& user) {
  INIT_GUARD();

  // EB(([this, &user]() {
  context_->user = user;
  context_->store.Reset();

  auto result = context_->data_adapter->GetDataSync(context_->user);
  context_->store.SetValuesFromDataAdapterResult(result);

  context_->store.Finalize();

  AsyncHelper::RunInBackground([this, result]() {
    context_->data_adapter->GetDataAsync(
        context_->user,
        result,
        [](const std::optional<DataAdapterResult>& result) {
          // noop
        }
        );
  });
  // }));
}

void StatsigClient::UpdateUserAsync(
    const StatsigUser& user,
    const std::function<void()>& callback
    ) {
  INIT_GUARD();

  // EB(([this, &user, &callback]() {
  context_->user = user;
  context_->store.Reset();

  auto result = context_->data_adapter->GetDataSync(context_->user);
  context_->store.SetValuesFromDataAdapterResult(result);

  const auto initiator = context_->user;

  const auto inner_callback = [this, initiator, callback]
  (std::optional<DataAdapterResult> result) {

    const auto current_user = context_->user;
    if (internal::AreUsersEqual(initiator, current_user)) {
      context_->store.SetValuesFromDataAdapterResult(result);
    }

    context_->store.Finalize();
    callback();
  };

  context_->data_adapter->GetDataAsync(
      initiator,
      result,
      inner_callback
      );
  // }));
}

void StatsigClient::Shutdown() {
  INIT_GUARD();

  EB(([this]() {
    context_->logger.Shutdown();
    context_.reset();
    }));
}

void StatsigClient::Flush() {
  INIT_GUARD();

  EB(([this]() {
    context_->logger.Flush();
    }));
}

void StatsigClient::LogEvent(const StatsigEvent& event) {
  INIT_GUARD();

  EB(([this, &event]() {
    context_->logger.Enqueue(internal::InternalizeEvent(event, context_->user));
    }));
}

bool StatsigClient::CheckGate(const std::string& gate_name) {
  INIT_GUARD(false);

  auto gate = GetFeatureGate(gate_name);
  return gate.GetValue();
}

FeatureGate StatsigClient::GetFeatureGate(const std::string& gate_name) {
  FeatureGate result(gate_name, internal::evaluation_details::Uninitialized());
  INIT_GUARD(result);

  // EB(([this, &gate_name, &result]() {
  auto gate = context_->store.GetGate(gate_name);

  context_->logger.Enqueue(
      internal::MakeGateExposure(
          gate_name,
          context_->user,
          gate
          )
      );

  result = FeatureGate(
      gate_name,
      UNWRAP(gate.evaluation, rule_id),
      gate.details,
      UNWRAP(gate.evaluation, value)
      );
  // }));

  return result;
}

DynamicConfig StatsigClient::GetDynamicConfig(const std::string& config_name) {
  DynamicConfig result(config_name,
                       internal::evaluation_details::Uninitialized());
  INIT_GUARD(result);

  //  EB(([this, &config_name, &result]() {
  auto config = context_->store.GetConfig(config_name);

  context_->logger.Enqueue(
      internal::MakeConfigExposure(
          config_name,
          context_->user,
          config
          )
      );

  result = DynamicConfig(
      config_name,
      UNWRAP(config.evaluation, rule_id),
      config.details,
      UNWRAP(config.evaluation, value)
      );
  //  }));

  return result;
}

Experiment StatsigClient::GetExperiment(const std::string& experiment_name) {
  Experiment result(experiment_name,
                    internal::evaluation_details::Uninitialized());
  INIT_GUARD(result);

  EB(([this, &experiment_name, &result]() {
    auto experiment = context_->store.GetConfig(experiment_name);
    context_->logger.Enqueue(
      internal::MakeConfigExposure(
        experiment_name,
        context_->user,
        experiment
      )
    );

    result = Experiment(
      experiment_name,
      UNWRAP(experiment.evaluation, rule_id),
      experiment.details,
      UNWRAP(experiment.evaluation, value)
    );
    }));

  return result;
}

Layer StatsigClient::GetLayer(const std::string& layer_name) {
  Layer result(layer_name, internal::evaluation_details::Uninitialized());
  INIT_GUARD(result);

  EB(([this, &layer_name, &result]() {
    auto logger = &context_->logger;
    auto user = context_->user;
    auto layer = context_->store.GetLayer(layer_name);

    auto log_exposure = [layer_name, layer, user, logger](const std::string &
      param_name) {
    auto expo = internal::MakeLayerParamExposure(
      layer_name,
      param_name,
      user,
      layer
    );
    logger->Enqueue(expo);
    };

    result = Layer(
      layer_name,
      UNWRAP(layer.evaluation, rule_id),
      layer.details,
      UNWRAP(layer.evaluation, value),
      log_exposure
    );
    }));

  return result;
}

bool StatsigClient::EnsureInitialized(const char* caller) {
  if (context_ != nullptr && context_->sdk_key.length() > 0) {
    return true;
  }

  std::cerr << "[Statsig]: Call made to StatsigClient::" << caller <<
      " before StatsigClient::Initialize" << std::endl;
  return false;
}

}
