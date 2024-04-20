#include "statsig/statsig_client.h"

#include <memory>

#include "statsig/statsig_event.h"
#include "statsig_context.hpp"
#include "evaluation_details_internal.hpp"
#include "statsig/compat/async/async_helper.hpp"
#include "statsig/compat/primitives/string.hpp"

#define INIT_GUARD(result) do { if (!EnsureInitialized(__func__)) { return result; }} while(0)
#define EB_WITH_TAG(tag, task) context_->err_boundary.Capture((tag), (task))
#define EB(task) EB_WITH_TAG(__func__, task)
#define USE_CTX(weak_ctx, shared_ctx) \
    auto shared_ctx = weak_ctx.lock();       \
    if (!shared_ctx) return

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

StatsigResultCode StatsigClient::InitializeSync(
    const String& sdk_key,
    const std::optional<StatsigUser>& user,
    const std::optional<StatsigOptions>& options) {
  const auto actual_key = FromCompat(sdk_key);
  if (actual_key.empty()) {
    return InvalidSdkKey;
  }

  return EB(([this, &actual_key, &user, &options] {
    context_ = std::make_shared<StatsigContext>(actual_key, user, options);
    return UpdateUserSync(context_->user);
    }));
}

void StatsigClient::InitializeAsync(
    const String& sdk_key,
    const std::function<void(StatsigResultCode)>& callback,
    const std::optional<StatsigUser>& user,
    const std::optional<StatsigOptions>& options) {
  const auto actual_key = FromCompat(sdk_key);
  if (actual_key.empty()) {
    callback(InvalidSdkKey);
    return;
  }

  EB(([this, &actual_key, &user, &options, &callback]() {
    context_ = std::make_shared<StatsigContext>(actual_key, user, options);
    UpdateUserAsync(context_->user, callback);
    return Ok;
    }));
}

StatsigResultCode StatsigClient::UpdateUserSync(const StatsigUser& user) {
  INIT_GUARD(ClientUninitialized);

  const auto tag = __func__;
  return EB(([this, &user, tag] {
    context_->user = user;
    context_->store.Reset();

    auto result = context_->data_adapter->GetDataSync(context_->user);
    if (result.code == Ok) {
    context_->store.SetValuesFromDataAdapterResult(result.value);
    }
    context_->store.Finalize();

    context_->err_boundary.ReportBadResult(tag, result.code, result.extra);

    std::weak_ptr<StatsigContext> weak_ctx = context_;
    AsyncHelper::RunInBackground([weak_ctx, result]() {
      USE_CTX(weak_ctx, shared_ctx);

      shared_ctx->data_adapter->GetDataAsync(
        shared_ctx->user,
        result.value,
        [](auto) {}
      );
      });

    return result.code;
    }));
}

void StatsigClient::UpdateUserAsync(
    const StatsigUser& user,
    const std::function<void(StatsigResultCode)>& callback
    ) {
  INIT_GUARD();
  const auto tag = __func__;
  EB(([this, &user, &callback, tag] {
    context_->user = user;
    context_->store.Reset();

    auto result = context_->data_adapter->GetDataSync(context_->user);
    if (result.code == Ok) {
    context_->store.SetValuesFromDataAdapterResult(result.value);
    }

    const auto initiator = context_->user;
    std::weak_ptr<StatsigContext> weak_ctx = context_;
    AsyncHelper::RunInBackground([weak_ctx, tag, initiator, result, callback]()
      {
      USE_CTX(weak_ctx, shared_ctx);

      const auto inner_callback =
      [weak_ctx, tag, initiator, callback](auto result) {
      USE_CTX(weak_ctx, shared_ctx);

      const auto current_user = shared_ctx->user;
      if (result.code == Ok && internal::AreUsersEqual(initiator, current_user))
      {
      shared_ctx->store.SetValuesFromDataAdapterResult(result.value);
      }

      shared_ctx->store.Finalize();
      shared_ctx->err_boundary.ReportBadResult(tag, result.code, result.extra);
      callback(result.code);
      };

      shared_ctx->data_adapter->GetDataAsync(
        initiator,
        result.value,
        inner_callback
      );
      });

    return Ok;
    }));
}

void StatsigClient::Shutdown() {
  INIT_GUARD();

  EB(([this]() {
    context_->logger.Shutdown();
    context_.reset();

    return Ok;
    }));
}

void StatsigClient::Flush() {
  INIT_GUARD();

  EB(([this]() {
    context_->logger.Flush();

    return Ok;
    }));
}

void StatsigClient::LogEvent(const StatsigEvent& event) {
  INIT_GUARD();

  EB(([this, &event]() {
    context_->logger.Enqueue(internal::InternalizeEvent(event, context_->user));

    return Ok;
    }));
}

bool StatsigClient::CheckGate(const String& gate_name) {
  INIT_GUARD(false);

  auto gate = GetFeatureGate(gate_name);
  return gate.GetValue();
}

FeatureGate StatsigClient::GetFeatureGate(const String& gate_name) {
  FeatureGate result(gate_name, internal::evaluation_details::Uninitialized());
  INIT_GUARD(result);

  EB(([this, &gate_name, &result]() {
    const auto gate_name_actual = FromCompat(gate_name);
    const auto gate = context_->store.GetGate(gate_name_actual);

    context_->logger.Enqueue(
      internal::MakeGateExposure(
        gate_name_actual,
        context_->user,
        gate
      )
    );

    result = FeatureGate(
      gate_name,
      ToCompat(UNWRAP(gate.evaluation, rule_id)),
      gate.details,
      UNWRAP(gate.evaluation, value)
    );

    return Ok;
    }));

  return result;
}

DynamicConfig StatsigClient::GetDynamicConfig(const String& config_name) {
  DynamicConfig result(config_name,
                       internal::evaluation_details::Uninitialized());
  INIT_GUARD(result);

  EB(([this, &config_name, &result] {
    const auto config_name_actual = FromCompat(config_name);
    const auto config = context_->store.GetConfig(config_name_actual);

    context_->logger.Enqueue(
      internal::MakeConfigExposure(
        config_name_actual,
        context_->user,
        config
      )
    );

    result = DynamicConfig(
      config_name,
      ToCompat(UNWRAP(config.evaluation, rule_id)),
      config.details,
      UNWRAP(config.evaluation, value)
    );

    return Ok;
    }));

  return result;
}

Experiment StatsigClient::GetExperiment(const String& experiment_name) {
  Experiment result(experiment_name,
                    internal::evaluation_details::Uninitialized());
  INIT_GUARD(result);

  EB(([this, &experiment_name, &result]() {
    const auto exp_name_actual = FromCompat(experiment_name);
    auto experiment = context_->store.GetConfig(exp_name_actual);
    context_->logger.Enqueue(
      internal::MakeConfigExposure(
        exp_name_actual,
        context_->user,
        experiment
      )
    );

    result = Experiment(
      experiment_name,
      ToCompat(UNWRAP(experiment.evaluation, rule_id)),
      experiment.details,
      UNWRAP(experiment.evaluation, value)
    );

    return Ok;
    }));

  return result;
}

Layer StatsigClient::GetLayer(const String& layer_name) {
  Layer result(layer_name, internal::evaluation_details::Uninitialized());
  INIT_GUARD(result);

  EB(([this, &layer_name, &result] {
    auto logger = &context_->logger;
    auto user = context_->user;

    const auto layer_name_actual = FromCompat(layer_name);
    const auto layer = context_->store.GetLayer(layer_name_actual);

    auto log_exposure = [layer_name_actual, layer, user, logger](const std::
      string &
      param_name) {
    auto expo = internal::MakeLayerParamExposure(
      layer_name_actual,
      param_name,
      user,
      layer
    );
    logger->Enqueue(expo);
    };

    result = Layer(
      layer_name,
      ToCompat(UNWRAP(layer.evaluation, rule_id)),
      layer.details,
      UNWRAP(layer.evaluation, value),
      log_exposure
    );

    return Ok;
    }));

  return result;
}

bool StatsigClient::EnsureInitialized(const char* caller) {
  if (context_ != nullptr && !context_->sdk_key.empty()) {
    return true;
  }

  std::cerr << "[Statsig]: Call made to StatsigClient::" << caller <<
      " before StatsigClient::Initialize" << std::endl;
  return false;
}

}
