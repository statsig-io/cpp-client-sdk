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
}

void StatsigClient::Shutdown() {

}

void StatsigClient::UpdateUser(StatsigUser *user) {
//  this->context_.user = user;
  fetch_and_save_values();
}

bool StatsigClient::CheckGate(const std::string &gate_name) {
  if (!this->context_.has_value()) {
    return false;
  }

  auto gate = this->context_->store->GetGate(gate_name);

  return this->context_.has_value();
}

void StatsigClient::fetch_and_save_values() {
  auto context = this->context_;

  context->network->fetch_for_user(context->user);
}

}


