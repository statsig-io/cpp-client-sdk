#pragma once

namespace statsig::internal::markers {

namespace /* private */ {
using string = std::string;
}


struct Base: public JsonObject {
  explicit Base(const string& action, const string &key) {
    emplace("action", action);
    emplace("key", key);
    emplace("timestamp", Time::now());
  }

  virtual ~Base() = default;
};

struct OverallStart : public Base {
  explicit OverallStart() :
      Base{"start", "overall"} {}
};

struct OverallEnd : public Base {
  const bool success;
  const EvaluationDetails details;

  explicit OverallEnd(
      bool is_successful,
      EvaluationDetails evaluation_details
  ) :
      Base{"end", "overall"},
      success(is_successful),
      details(std::move(std::move(evaluation_details))) {}
};

}