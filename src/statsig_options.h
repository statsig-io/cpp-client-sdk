#ifndef STATSIG_OPTIONS_H
#define STATSIG_OPTIONS_H

#include <string>

namespace statsig {
    class StatsigOptions {
    public:
        explicit StatsigOptions(std::string api = "https://statsigapi.net/v1/") : api_(std::move(api)) {}

        std::string getApi();

    private:
        std::string api_;
    };
}


#endif //STATSIG_OPTIONS_H
