#include <iostream>

#include "statsig.h"
#include "statsig_client.h"

namespace statsig {
    namespace {
        StatsigClient *_instance;
    }

    void initialize(const std::string &sdk_key, StatsigUser *user, StatsigOptions *options) {
      _instance = new StatsigClient(sdk_key, user, options);
      _instance->initialize();
    }

    void shutdown() {
      _instance->shutdown();
    }

    bool check_gate(const std::string& gate_name, bool default_value) {
      _instance->check_gate(gate_name, default_value);
    }
}