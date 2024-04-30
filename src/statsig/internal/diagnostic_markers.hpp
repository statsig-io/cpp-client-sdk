#pragma once

namespace statsig::internal::markers {

namespace /* private */ {
using string = std::string;

inline std::string Snippet(const std::string &str) {
  if (str.length() > 100) {
    return str.substr(0, 100);
  } else {
    return str;
  }
}

}

struct Base : public JsonObject {
  explicit Base(
      const string &action,
      const string &key,
      const std::optional<string> &step = std::nullopt
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
      StatsigResultCode result_code,
      EvaluationDetails evaluation_details
  ) : Base{"end", "overall"} {
    emplace("success", result_code == Ok);

    std::unordered_map<string, JsonValue> details{
        {"reason", evaluation_details.reason},
    };
    if (evaluation_details.reason != "NoValues") {
      details["lcut"] = evaluation_details.lcut;
      details["receivedAt"] = evaluation_details.received_at;
    }
    emplace("evaluationDetails", details);

    if (result_code != Ok) {
      const auto err = ResultCodeToString(result_code);
      emplace("error", std::unordered_map<string, JsonValue>{
          {"name", err},
          {"message", err},
      });
    }
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
      int attempt, const string &key = "initialize"
  ) : NetworkBase{"start", attempt, key} {}
};

struct NetworkEnd : public NetworkBase {
  explicit NetworkEnd(
      int attempt,
      int status,
      const string &response_text,
      const string &sdk_region,
      const std::optional<string> &error,
      const string &key = "initialize"
  ) : NetworkBase{"end", attempt, key} {
    const auto is_success = status >= 200 && status < 300;
    emplace("success", status >= 200 && status < 300);

    if (status > 0) {
      emplace("statusCode", status);
    }

    if (!sdk_region.empty()) {
      emplace("sdkRegion", sdk_region);
    }

    if (error.has_value()) {
      emplace("error", std::unordered_map<string, JsonValue>{
          {"name", error.value()},
          {"message", error.value()},
      });
    } else if (!is_success) {
      const auto start = Snippet(response_text);
      emplace("error", std::unordered_map<string, JsonValue>{
          {"name", start},
          {"message", start},
      });
    }
  }
};

/**
 * Process Markers
 */

struct ProcessBase : public Base {
  explicit ProcessBase(const string &action, const string &key = "initialize")
      : Base{action, key, "process"} {}
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