#pragma once

#include <utility>
#include <execinfo.h>

#include "constants.h"

namespace statsig {

class ErrorBoundary {
 public:
  explicit ErrorBoundary(string &sdk_key) : sdk_key_(sdk_key) {}

  inline static string eb_api = kDefaultApi;

  void Capture(
      const string &tag,
      const function<void()> &task
  ) {
    try {
      task();
    } catch (const exception &error) {
      cerr << "[Statsig]: An unexpected exception occurred. " << error.what() << endl;
      LogError(tag, error.what());
    }
  }

 private:
  string &sdk_key_;
  set<string> seen_;

  void LogError(
      const string &tag,
      const string &error
  ) {
    try {
      if (seen_.find(error) != seen_.end()) {
        return;
      }

      seen_.insert(error);

      json body = {
          {"exception", error},
          {"info", GetStackTrace()},
          {"tag", tag}
      };

      httplib::Headers headers = {
          {"STATSIG-API-KEY", sdk_key_},
          {"STATSIG-SDK-TYPE", kSdkType},
          {"STATSIG-SDK-VERSION", kSdkVersion}
      };

      httplib::Client(eb_api)
          .Post(
              "/v1/sdk_exception",
              headers,
              json(body).dump(),
              kContentTypeJson
          );
    } catch (exception &_) {
      // noop
    }
  }

  static vector<string> GetStackTrace() {
    const int maxFrames = 128;
    void *stackTrace[maxFrames];
    int stackSize = backtrace(stackTrace, maxFrames);
    char **symbols = backtrace_symbols(stackTrace, stackSize);

    vector < string > trace;

    if (symbols == nullptr) {
      return trace;
    }

    for (int i = 0; i < stackSize; ++i) {
      trace.emplace_back(symbols[i]);
    }

    free(symbols);
    return trace;
  }

};

}