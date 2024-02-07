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
  this->context_ = StatsigContext(sdk_key, user, options);

  set_values_from_network();
}

void StatsigClient::Shutdown() {

}

void StatsigClient::UpdateUser(StatsigUser *user) {
//  this->context_.user = user;
}

bool StatsigClient::CheckGate(const std::string &gate_name) {
  if (!this->context_.has_value()) {
    return false;
  }

  auto gate = this->context_->store->GetGate(gate_name);

  // log

  return gate;
}

void StatsigClient::set_values_from_network() {
  auto ctx = this->context_;
  auto response = ctx->network->FetchValues(&ctx->user);
  if (response.has_value()) {
    ctx->store->SetAndCacheValues(response.value());
  }
}

}


