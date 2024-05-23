#pragma once

#include <algorithm>
#include <numeric>
#include <sstream>

#include "../statsig_options.h"
#include "../statsig_user.h"
#include "hashing.hpp"
#include "unordered_map_util.hpp"
#include "statsig_compat/primitives/string.hpp"

#define UNWRAP_STR(opt) ((opt).has_value() ? (opt).value() : (""))

namespace statsig::internal {


inline StatsigUser NormalizeUser(
    const StatsigUser &user,
    const StatsigOptions &options
) {
  StatsigUser copy = {
      user.user_id,
      user.custom_ids,
      user.email,
      user.ip,
      user.user_agent,
      user.country,
      user.locale,
      user.app_version,
      user.custom,
      user.private_attributes,
      options.environment
  };

  return copy;
}

inline std::string GetFullUserHash(const StatsigUser &user) {
  std::unordered_map<std::string, std::string> pairs{
      {"u", UNWRAP_STR(user.user_id)},
      {"ci", CreateSortedMapString(user.custom_ids)},
      {"e", UNWRAP_STR(user.email)},
      {"ip", UNWRAP_STR(user.ip)},
      {"ua", UNWRAP_STR(user.user_agent)},
      {"ct", UNWRAP_STR(user.country)},
      {"lc", UNWRAP_STR(user.locale)},
      {"av", UNWRAP_STR(user.app_version)},
      {"cst", CreateSortedMapString(user.custom)},
      {"pa", CreateSortedMapString(user.private_attributes)},
      {"tr", ""}
  };

  if (user.statsig_environment.has_value()) {
    pairs["tr"] = user.statsig_environment.value().tier.value_or("");
  }

  std::ostringstream oss;
  for (const auto &pair : pairs) {
    oss << pair.first << ":" << pair.second << "-";
  }

  return hashing::DJB2(oss.str());
}

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
  return GetFullUserHash(left) == GetFullUserHash(right);
}

}
