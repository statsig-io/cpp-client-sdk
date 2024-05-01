#pragma once

#include "statsig_compat/primitives/json_value.hpp"

namespace statsig::internal::markers {

struct Base {
  using string = std::string;

  explicit Base(
      const string &action,
      const string &key,
      const std::optional<string> &step = std::nullopt
  ) {
    data_ = EmptyJsonObject();

    AddValue("action", StringToJsonValue(action));
    AddValue("key", StringToJsonValue(key));
    AddValue("timestamp", TimeToJsonValue(Time::now()));

    if (step.has_value()) {
      AddValue("step", StringToJsonValue(step.value()));
    }
  }

  virtual ~Base() = default;

  JsonValue GetData() const {
    return JsonObjectToJsonValue(data_);
  }

 protected:
  JsonObject data_;

  void AddValue(const std::string &key, const JsonValue &value) {
    AddToJsonObject(data_, key, value);
  }

  static std::string Snippet(const std::string &str) {
    if (str.length() > 100) {
      return str.substr(0, 100);
    }
    return str;
  }
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
      const StatsigResultCode result_code,
      const EvaluationDetails &evaluation_details
  )
      : Base{"end", "overall"} {
    AddValue("success", BoolToJsonValue(result_code == Ok));

    std::unordered_map<string, JsonValue> details{
        {"reason", CompatStringToJsonValue(evaluation_details.reason)},
    };

    if (evaluation_details.reason != "NoValues") {
      details["lcut"] = JsonValueFromNumber(evaluation_details.lcut);
      details["receivedAt"] = JsonValueFromNumber(
          evaluation_details.received_at);
    }

    AddValue("evaluationDetails", JsonValueMapToJsonValue(details));

    if (result_code != Ok) {
      const auto err = StringToJsonValue(ResultCodeToString(result_code));
      const std::unordered_map<string, JsonValue> err_data{
          {"name", err},
          {"message", err},
      };
      AddValue("error", JsonValueMapToJsonValue(err_data));
    }
  }
};

/**
 * Network Markers
 */

struct NetworkBase : public Base {
  explicit NetworkBase(
      const string &action,
      const int attempt,
      const string &key
  )
      : Base{action, key, "network_request"} {
    AddValue("attempt", IntToJsonValue(attempt));
  }
};

struct NetworkStart : public NetworkBase {
  explicit NetworkStart(
      const int attempt,
      const string &key = "initialize"
  )
      : NetworkBase{"start", attempt, key} {}
};

struct NetworkEnd : public NetworkBase {
  explicit NetworkEnd(
      const int attempt,
      const int status,
      const string &response_text,
      const string &sdk_region,
      const std::optional<string> &error,
      const string &key = "initialize"
  )
      : NetworkBase{"end", attempt, key} {
    const auto is_success = status >= 200 && status < 300;
    AddValue("success", BoolToJsonValue(status >= 200 && status < 300));

    if (status > 0) {
      AddValue("statusCode", IntToJsonValue(status));
    }

    if (!sdk_region.empty()) {
      AddValue("sdkRegion", StringToJsonValue(sdk_region));
    }

    if (error.has_value()) {
      const auto err = StringToJsonValue(error.value());
      const std::unordered_map<string, JsonValue> err_data{
          {"name", err},
          {"message", err},
      };
      AddValue("error", JsonValueMapToJsonValue(err_data));
    } else if (!is_success) {
      const auto snippet = StringToJsonValue(Snippet(response_text));
      const std::unordered_map<string, JsonValue> err_data{
          {"name", snippet},
          {"message", snippet},
      };
      AddValue("error", JsonValueMapToJsonValue(err_data));
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
  explicit ProcessStart()
      : ProcessBase{"start"} {}
};

struct ProcessEnd : public ProcessBase {
  explicit ProcessEnd(const bool is_successful)
      : ProcessBase{"end"} {
    AddValue("success", BoolToJsonValue(is_successful));
  }

};

}
