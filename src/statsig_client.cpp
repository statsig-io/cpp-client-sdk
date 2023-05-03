#include "statsig_client.h"

namespace statsig {
    void StatsigClient::initialize() {
      fetch_and_save_values();
    }

    void StatsigClient::shutdown() {

    }

    void StatsigClient::update_user(StatsigUser *user) {
      this->user_ = user;
      fetch_and_save_values();
    }

    bool StatsigClient::check_gate(const std::string &gate_name, bool default_value) {
      return false;
    }

    void StatsigClient::fetch_and_save_values() {
      network_->fetch_for_user(user_);
    }
}


