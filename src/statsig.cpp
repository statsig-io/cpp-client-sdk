#include <iostream>
#include "statsig_client.h"

namespace statsig {
    namespace {
        StatsigClient *_instance;
    }

    void initialize(const std::string &sdk_key) {
      _instance = new StatsigClient(sdk_key, nullptr);
    }

    void shutdown() {
      _instance->shutdown();
    }

    bool check_gate(const std::string& gate_name, bool default_value = false) {
      _instance->check_gate(gate_name, default_value);
    }
}