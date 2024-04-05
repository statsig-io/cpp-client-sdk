#pragma once

#include <nlohmann/json.hpp>

#include "../initialize_request_args.h"

namespace statsig::data_types::initialize_request_args {

std::string Serialize(const internal::InitializeRequestArgs &args) {
  auto j = nlohmann::json{
      {"hash", args.hash},
      {"user", statsig_user::ToJson(args.user)},
  };

  return j.dump();
}

internal::InitializeRequestArgs Deserialize(const std::string &input) {
  auto j = nlohmann::json::parse(input);
  internal::InitializeRequestArgs res;
//  j.at("source").get_to(res.source);
//  j.at("data").get_to(res.data);
//  j.at("receivedAt").get_to(res.receivedAt);
  return res;
}

}