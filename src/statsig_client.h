#include <iostream>
#include <utility>
#include "statsig_user.h"

#ifndef STATSIG_CLIENT_H
#define STATSIG_CLIENT_H

namespace statsig {
    class StatsigClient {
    public:
        explicit StatsigClient(std::string sdk_key, StatsigUser *user) : sdk_key_(std::move(sdk_key)),
                                                                         user_(user) {};

        void shutdown();

        void update_user(StatsigUser *user);

        bool check_gate(const std::string& gate_name, bool default_value);

//        void get_config();
//
//        void get_experiment();
//
//        void get_layer();

    private:
        std::string sdk_key_;
        StatsigUser *user_;
    };

}

#endif //STATSIG_CLIENT_H
