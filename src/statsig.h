#ifndef STATSIG_H
#define STATSIG_H

#include "statsig_user.h"
#include "statsig_options.h"

namespace statsig {
    void initialize(const std::string &sdk_key, StatsigUser *user = nullptr, StatsigOptions *options = nullptr);

    void shutdown();

    bool check_gate(const std::string& gate_name, bool default_value = false);
}

#endif //STATSIG_H
