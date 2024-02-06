#pragma once

#include <string>

namespace statsig {

struct StatsigOptions {
  std::string api;
};
//    class StatsigOptions {
//    public:
//        explicit StatsigOptions(std::string api = "https://statsigapi.net/v1/") : api_(std::move(api)) {}
//
//        std::string getApi();
//
//    private:
//        std::string api_;
//    };
}
