#include <map>
#include <string>
#include <utility>

#ifndef STATSIG_USER_H
#define STATSIG_USER_H

namespace statsig {
    class StatsigUser {
    public:
        explicit StatsigUser(std::string user_id) : user_id_(std::move(user_id)) {}

        explicit StatsigUser(const std::map<std::string, std::string> &custom_ids)
            : custom_ids_(custom_ids) {}

        std::string getUserID();

        std::map<std::string, std::string> getCustomIDs();

    private:
        std::string user_id_;
        std::map<std::string, std::string> custom_ids_;

    };


}

#endif //STATSIG_USER_H
