#pragma once

#include "hashing.hpp"
#include "../statsig_user.h"
#include "statsig_compat/primitives/string.hpp"

namespace statsig::internal {

inline std::string MakeCacheKey(
    const std::string &sdk_key,
    const StatsigUser &user) {

  auto pairs = GetKeyValuePairs(user.custom_ids);
  String custom_ids;

  if (!pairs.empty()) {
    sort(
        pairs.begin(), pairs.end(),
        [](const auto &left, const auto &right) {
          return left.first < right.first;
        });

    auto const [k1, v1] = pairs[0];
    custom_ids = accumulate(
        next(pairs.begin()), pairs.end(), k1 + "-" + v1,
        [](const String &acc, const std::pair<String, String> &p) {
          return acc + "," + p.first + "-" + p.second;
        });
  }

  std::vector parts{
      "uid:" + user.user_id.value_or(""),
      "cids:" + custom_ids,
      "k:" + ToCompat(sdk_key)};

  const auto result = accumulate(
      next(parts.begin()), parts.end(), parts[0],
      [](const String &acc, const String &s) {
        return acc + "|" + s;
      });

  return hashing::DJB2(FromCompat(result));
}

inline bool AreUsersEqual(const StatsigUser &left, const StatsigUser &right) {
  if (left.user_id != right.user_id) {
    return false;
  }

  return true;
}

}
