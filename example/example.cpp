#ifndef STATSIG_UNREAL

#include <statsig/statsig.h>
#include <iostream>

using namespace statsig;

StatsigUser user;
StatsigClient client;

void SimpleGateCheck() {
  user.user_id = "user_simple_gate_check";

  client.Initialize("client-rfLvYGag3eyU0jYW5zcIJTQip7GXxSrhOFN69IGMjvq", user);

  auto a_gate = client.CheckGate("a_gate");
  std::cout << "a_gate: " << (a_gate ? "Pass" : "Fail") << std::endl;

  client.Shutdown();
}

void SessionDelayedNetworkInit() {
  user.user_id = "user_session_delayed_network_init";

  StatsigOptions opts;
  opts.providers = {
      new statsig::evaluations_data_providers::LocalFileCacheEvaluationsDataProvider()
  };

  client.Initialize("client-rfLvYGag3eyU0jYW5zcIJTQip7GXxSrhOFN69IGMjvq", user, opts);

  auto a_gate = client.CheckGate("a_gate");
  std::cout << "a_gate: " << (a_gate ? "Pass" : "Fail") << std::endl;

  client.Shutdown();
}

int main() {
  SimpleGateCheck();
  SessionDelayedNetworkInit();
}

#endif