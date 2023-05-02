#include "statsig_user.h"

namespace statsig {
    std::string StatsigUser::getUserID() {
      return this->user_id_;
    }

    std::map<std::string, std::string> StatsigUser::getCustomIDs() {
      return this->custom_ids_;
    }
}
