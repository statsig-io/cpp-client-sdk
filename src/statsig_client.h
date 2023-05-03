#ifndef STATSIG_CLIENT_H
#define STATSIG_CLIENT_H

#include "statsig_user.h"
#include "statsig_options.h"
#include "network_service.h"
#include "statsig_logger.h"

namespace statsig {
    class StatsigClient {
    public:
        explicit StatsigClient(std::string sdk_key,
                               StatsigUser *user,
                               StatsigOptions *options)
            : sdk_key_(std::move(sdk_key)),
              user_(user) {
          network_ = new NetworkService(sdk_key_, options);
          logger_ = new StatsigLogger();
        };

        void initialize();

        void shutdown();

        void update_user(StatsigUser *user);

        bool check_gate(const std::string &gate_name, bool default_value);

//        void get_config();
//
//        void get_experiment();
//
//        void get_layer();

    private:
        std::string sdk_key_;
        StatsigUser *user_;
        NetworkService *network_;
        StatsigLogger *logger_;

        void fetch_and_save_values();
    };

}

#endif //STATSIG_CLIENT_H
