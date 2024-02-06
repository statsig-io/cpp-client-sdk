#include "statsig_client.h"

namespace statsig {

StatsigClient &StatsigClient::Shared() {
  static StatsigClient inst;
  return inst;
}

void StatsigClient::Initialize(
    const std::string &sdk_key,
    StatsigUser *user,
    StatsigOptions *options
) {
  StatsigContext ctx = StatsigContext(sdk_key, options, user);
  this->context_ = ctx;

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
  ctx->network->FetchValues(ctx->user);
}

}


