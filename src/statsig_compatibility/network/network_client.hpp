#pragma once

#include "httplib.h"
#include <functional>

namespace statsig::internal {

struct HttpResponse {
  const std::string text;
  const int status = -1;
};

class NetworkClient {
 public:
  static void Post(
      const std::string &api,
      const std::string &path,
      const std::unordered_map<std::string, std::string> &headers,
      const std::string &body,
      const std::function<void(HttpResponse)>& callback
  ) {
    httplib::Client client(api);
    client.set_compress(path == constants::kEndpointLogEvent);

    httplib::Headers compat_headers;
    for (const auto &kv : headers) {
      compat_headers.insert(kv);
    }

    httplib::Result result = client.Post(
        path,
        compat_headers,
        body,
        constants::kContentTypeJson
    );

    callback(HttpResponse{result->body, result->status});
  }
};

}



