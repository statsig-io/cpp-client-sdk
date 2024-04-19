#include "../public/statsig_client.h"

#include <memory>
#include <utility>

#include "public/statsig_event.h"
#include "statsig_context.hpp"
#include "evaluation_details_internal.hpp"
#include "compat/async/async_helper.hpp"

#define INIT_GUARD(result) do { if (!EnsureInitialized(__func__)) { return result; }} while(0)
#define EB_WITH_TAG(tag, task) context_->err_boundary.Capture((tag), (task))
#define EB(task) EB_WITH_TAG(__func__, task)
#define USE_CTX(weak_ctx, shared_ctx) \
    auto shared_ctx = weak_ctx.lock();       \
    if (!shared_ctx) return

namespace statsig {

StatsigClient &StatsigClient::Shared() {
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
    const std::string &sdk_key,
    const std::optional<StatsigUser> &user,
    const std::optional<StatsigOptions> &options) {
  if (sdk_key.empty()) {
    return InvalidSdkKey;
  }

  auto code = Ok;

//  EB(([this, &sdk_key, &user, &options, &code] {
  context_ = std::make_shared<StatsigContext>(sdk_key, user, options);
  code = UpdateUserSync(context_->user);
//  }));

  return code;
}

void StatsigClient::InitializeAsync(
    const std::string &sdk_key,
    const std::function<void(StatsigResultCode)> &callback,
    const std::optional<StatsigUser> &user,
    const std::optional<StatsigOptions> &options) {
  EB(([this, &sdk_key, &user, &options, &callback]() {
    context_ = std::make_shared<StatsigContext>(sdk_key, user, options);
    UpdateUserAsync(context_->user, callback);

    return Ok;
  }));
}

StatsigResultCode StatsigClient::UpdateUserSync(const StatsigUser &user) {
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
    const StatsigUser &user,
    const std::function<void(StatsigResultCode)> &callback
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
    AsyncHelper::RunInBackground([weak_ctx, tag, initiator, result, callback]() {
      USE_CTX(weak_ctx, shared_ctx);

      const auto inner_callback =
          [weak_ctx, tag, initiator, callback](auto result) {
            USE_CTX(weak_ctx, shared_ctx);

            const auto current_user = shared_ctx->user;
            if (result.code == Ok && internal::AreUsersEqual(initiator, current_user)) {
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

void StatsigClient::LogEvent(const StatsigEvent &event) {
  INIT_GUARD();

  EB(([this, &event]() {
    context_->logger.Enqueue(internal::InternalizeEvent(event, context_->user));

    return Ok;
  }));
}

bool StatsigClient::CheckGate(const std::string &gate_name) {
  INIT_GUARD(false);

  auto gate = GetFeatureGate(gate_name);
  return gate.GetValue();
}

FeatureGate StatsigClient::GetFeatureGate(const std::string &gate_name) {
  FeatureGate result(gate_name, internal::evaluation_details::Uninitialized());
  INIT_GUARD(result);

  EB(([this, &gate_name, &result]() {
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

    return Ok;
  }));

  return result;
}

DynamicConfig StatsigClient::GetDynamicConfig(const std::string &config_name) {
  DynamicConfig result(config_name,
                       internal::evaluation_details::Uninitialized());
  INIT_GUARD(result);

  EB(([this, &config_name, &result]() {
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

    return Ok;
  }));

  return result;
}

Experiment StatsigClient::GetExperiment(const std::string &experiment_name) {
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

    return Ok;
  }));

  return result;
}

Layer StatsigClient::GetLayer(const std::string &layer_name) {
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

    return Ok;
  }));

  return result;
}

bool StatsigClient::EnsureInitialized(const char *caller) {
  if (context_ != nullptr && !context_->sdk_key.empty()) {
    return true;
  }

  std::cerr << "[Statsig]: Call made to StatsigClient::" << caller <<
            " before StatsigClient::Initialize" << std::endl;
  return false;
}

}
