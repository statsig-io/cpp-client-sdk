#ifndef STATSIG_USER_H
#define STATSIG_USER_H

#include <string>
#include <unordered_map>

namespace statsig {
    class StatsigUser {
    public:
        explicit StatsigUser(std::string user_id) : user_id_(std::move(user_id)) {}

        explicit StatsigUser(const std::unordered_map<std::string, std::string> &custom_ids)
            : custom_ids_(custom_ids) {}

        std::string getUserID();

        std::unordered_map<std::string, std::string> getCustomIDs();

    private:
        std::string user_id_;
        std::unordered_map<std::string, std::string> custom_ids_;

    };


}

#endif //STATSIG_USER_H
