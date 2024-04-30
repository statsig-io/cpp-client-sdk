#pragma once

namespace statsig::internal::markers {

namespace /* private */ {
using string = std::string;
}

struct Base : public JsonObject {
  explicit Base(
      const string &action,
      const string &key,
      const std::optional<string>& step = std::nullopt
  ) {
    emplace("action", action);
    emplace("key", key);
    emplace("timestamp", Time::now());

    if (step.has_value()) {
      emplace("step", step.value());
    }
  }

  virtual ~Base() = default;
};

/**
 * Overall Initialize Markers
 */

struct OverallStart : public Base {
  explicit OverallStart() : Base{"start", "overall"} {}
};

struct OverallEnd : public Base {
  explicit OverallEnd(
      bool is_successful,
      EvaluationDetails evaluation_details
  ) : Base{"end", "overall"} {
    emplace("success", is_successful);
    emplace("evaluationDetails", std::unordered_map<string, JsonValue>{
        {"reason", evaluation_details.reason},
        {"lcut", evaluation_details.lcut},
        {"receivedAt", evaluation_details.received_at},
    });
  }
};

/**
 * Network Markers
 */

struct NetworkBase : public Base {
  explicit NetworkBase(
      const string &action,
      int attempt,
      const string &key
  ) : Base{action, key, "network_request"} {
    emplace("attempt", attempt);
  }
};

struct NetworkStart : public NetworkBase {
  explicit NetworkStart(
      int attempt, const string &key = "initialize")
      : NetworkBase{"start", attempt, key} {}
};

struct NetworkEnd : public NetworkBase {
  explicit NetworkEnd(
      int attempt,
      int status,
      const string &sdk_region,
      const std::optional<string> &error,
      const string &key = "initialize"
  ) : NetworkBase{"end", attempt, key} {
    emplace("success", status >= 200 && status < 300);
    emplace("statusCode", status);
    emplace("sdkRegion", sdk_region);

    if (error.has_value()) {
      emplace("error", std::unordered_map<string, JsonValue>{
          {"name", error.value()},
      });
    }
  }
};

/**
 * Process Markers
 */

struct ProcessBase : public Base {
  explicit ProcessBase(const string &action) : Base{action, "process"} {}
};

struct ProcessStart : public ProcessBase {
  explicit ProcessStart() : ProcessBase{"start"} {}
};

struct ProcessEnd : public ProcessBase {
  explicit ProcessEnd(bool is_successful) : ProcessBase{"end"} {
    emplace("success", is_successful);
  }

};

}