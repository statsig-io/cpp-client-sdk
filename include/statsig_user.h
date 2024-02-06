#pragma once

#include <string>
#include <unordered_map>

namespace statsig {

struct StatsigUser {
  std::string user_id;
};
//    class StatsigUser {
//    public:
//        explicit StatsigUser(std::string user_id) : user_id_(std::move(user_id)) {}
//
//        explicit StatsigUser(const std::unordered_map<std::string, std::string> &custom_ids)
//            : custom_ids_(custom_ids) {}
//
//        std::string getUserID();
//
//        std::unordered_map<std::string, std::string> getCustomIDs();
//
//    private:
//        std::string user_id_;
//        std::unordered_map<std::string, std::string> custom_ids_;
//
//    };


}

