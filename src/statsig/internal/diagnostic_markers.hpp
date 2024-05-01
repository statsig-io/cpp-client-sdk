#pragma once

#include "statsig_compat/primitives/json_value.hpp"

namespace statsig::internal::markers {

namespace /* private */ {
using string = std::string;

inline std::string Snippet(const std::string& str) {
  if (str.length() > 100) {
    return str.substr(0, 100);
  } else {
    return str;
  }
}

}

struct Base : public JsonObject {
  explicit Base(
      const string& action,
      const string& key,
      const std::optional<string>& step = std::nullopt
      ) {
    AddValue("action", action);
    AddValue("key", key);
    AddNumValue("timestamp", Time::now());

    if (step.has_value()) {
      AddValue("step", step.value());
    }
  }

  virtual ~Base() = default;

protected:
#ifdef STATSIG_UNREAL_PLUGIN
  void AddValue(std::string key, std::string value) {
    Get()->SetStringField(FString(key.c_str()), FString(value.c_str()));
  }

  void AddBoolValue(std::string key, bool value) {
    Get()->SetBoolField(FString(key.c_str()), value);
  }

  void AddNumValue(std::string key, long value) {
    Get()->SetNumberField(FString(key.c_str()), value);
  }

  void AddValue(std::string key, std::unordered_map<string, JsonValue>) {}
#else
  void AddValue(std::string key, JsonValue value) {
      emplace(key, value);
  }
#endif
};

/**
 * Overall Initialize Markers
 */

struct OverallStart : public Base {
  explicit OverallStart()
    : Base{"start", "overall"} {}
};

struct OverallEnd : public Base {
  explicit OverallEnd(
      StatsigResultCode result_code,
      EvaluationDetails evaluation_details
      )
    : Base{"end", "overall"} {
    AddBoolValue("success", result_code == Ok);

    std::unordered_map<string, JsonValue> details{
        {"reason", ToJsonValue(evaluation_details.reason)},
    };

    if (evaluation_details.reason != "NoValues") {
      details["lcut"] = JsonValueFromNumber(evaluation_details.lcut);
      details["receivedAt"] = JsonValueFromNumber(
          evaluation_details.received_at);
    }

    AddValue("evaluationDetails", details);

    if (result_code != Ok) {
      const auto err = StdStrToJsonValue(ResultCodeToString(result_code));
      AddValue("error", std::unordered_map<string, JsonValue>{
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
      const string& action,
      int attempt,
      const string& key
      )
    : Base{action, key, "network_request"} {
    AddNumValue("attempt", attempt);
  }
};

struct NetworkStart : public NetworkBase {
  explicit NetworkStart(
      int attempt, const string& key = "initialize"
      )
    : NetworkBase{"start", attempt, key} {}
};

struct NetworkEnd : public NetworkBase {
  explicit NetworkEnd(
      int attempt,
      int status,
      const string& response_text,
      const string& sdk_region,
      const std::optional<string>& error,
      const string& key = "initialize"
      )
    : NetworkBase{"end", attempt, key} {
    const auto is_success = status >= 200 && status < 300;
    AddBoolValue("success", status >= 200 && status < 300);

    if (status > 0) {
      AddNumValue("statusCode", status);
    }

    if (!sdk_region.empty()) {
      AddValue("sdkRegion", sdk_region);
    }

    if (error.has_value()) {
      const auto err = StdStrToJsonValue(error.value());
      AddValue("error", std::unordered_map<string, JsonValue>{
                   {"name", err},
                   {"message", err},
               });
    } else if (!is_success) {
      const auto start = StdStrToJsonValue(Snippet(response_text));
      AddValue("error", std::unordered_map<string, JsonValue>{
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
  explicit ProcessBase(const string& action, const string& key = "initialize")
    : Base{action, key, "process"} {}
};

struct ProcessStart : public ProcessBase {
  explicit ProcessStart()
    : ProcessBase{"start"} {}
};

struct ProcessEnd : public ProcessBase {
  explicit ProcessEnd(bool is_successful)
    : ProcessBase{"end"} {
    AddBoolValue("success", is_successful);
  }

};

}
