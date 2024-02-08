#pragma once

#include "hashing.hpp"

namespace statsig {

void to_json(json &j, const StatsigUser &u) {
  j = json{
      {"userID", u.user_id},
      {"customIDs", u.custom_ids},
  };
}

void from_json(const json &j, StatsigUser &u) {
  u.user_id = j.value("userID", "");
  u.custom_ids = j.value("customIDs", unordered_map<string, string>());
}

string MakeCacheKey(const string &sdk_key, const StatsigUser &user) {
  vector<pair<string, string>> pairs(user.custom_ids.begin(), user.custom_ids.end());

  string custom_ids;

  if (!pairs.empty()) {
    sort(
        pairs.begin(), pairs.end(),
        [](const auto &left, const auto &right) {
          return left.first < right.first;
        });

    auto const [k1, v1] = pairs[0];
    custom_ids = accumulate(
        next(pairs.begin()), pairs.end(), k1 + "-" + v1,
        [](const string &acc, const pair<string, string> &p) {
          return acc + "," + p.first + "-" + p.second;
        }
    );
  }

  vector<string> parts{
      "uid:" + user.user_id,
      "cids:" + custom_ids,
      "k:" + sdk_key
  };

  auto result = accumulate(
      next(parts.begin()), parts.end(), parts[0],
      [](const string &acc, const string &s) {
        return acc + "|" + s;
      }
  );

  return hashing::DJB2(result);
}

}