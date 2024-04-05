#pragma once

#ifdef STATSIG_WITH_HTTPLIB
#include "httplib.h"
#endif

namespace statsig::internal {

struct HttpResponse {
  const std::string text;
  const int status = -1;
};

class NetworkCompat {
 public:
  static HttpResponse Post(
      const std::string &api,
      const std::string &path,
      const std::unordered_map<std::string, std::string> &headers,
      const std::string &body
  ) {

#ifdef STATSIG_WITH_HTTPLIB

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

    return {result->body, result->status};

#else

    return {};

#endif
  }
};

}



