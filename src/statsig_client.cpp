#include "statsig_client.h"

namespace statsig {
    void StatsigClient::shutdown() {

    }

    void StatsigClient::update_user(StatsigUser *user) {
      this->user_ = user;
    }

    bool StatsigClient::check_gate(const std::string &gate_name, bool default_value) {
      return false;
    }
}


