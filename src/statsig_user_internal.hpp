#pragma once

#include "hashing.hpp"
#include "macros.hpp"
#include "statsig/statsig_user.h"

namespace statsig::internal {

std::string MakeCacheKey(
    const std::string &sdk_key,
    const StatsigUser &user
) {
  using string = std::string;

  std::vector<std::pair<string, string>> pairs(
      user.custom_ids.begin(),
      user.custom_ids.end()
  );

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
        [](const string &acc, const std::pair<string, string> &p) {
          return acc + "," + p.first + "-" + p.second;
        });
  }

  std::vector<string> parts{
      "uid:" + user.user_id,
      "cids:" + custom_ids,
      "k:" + sdk_key
  };

  auto result = accumulate(
      next(parts.begin()), parts.end(), parts[0],
      [](const string &acc, const string &s) {
        return acc + "|" + s;
      });

  return hashing::DJB2(result);
}

bool AreUsersEqual(const StatsigUser &left, const StatsigUser &right) {
  if (left.user_id != right.user_id) {
    return false;
  }

  return true;
}

}