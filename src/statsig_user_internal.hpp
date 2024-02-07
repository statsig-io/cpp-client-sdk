#pragma once

namespace statsig {

void to_json(json &j, const StatsigUser &u) {
  j = json{
      {"userID", u.user_id},
//      {"customIDs", u.cus},
  };
}

void from_json(const json &j, StatsigUser &u) {
  j.at("userID").get_to(u.user_id);
}

}