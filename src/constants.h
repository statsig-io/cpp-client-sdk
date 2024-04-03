#pragma once

namespace statsig::constants {

// Statsig Metadata
const char *kSdkType = "cpp-client";
const char *kSdkVersion = "0.0.1";

// HTTP Endpoints
const char *kEndpointInitialize = "/v1/initialize";
const char *kEndpointLogEvent = "/v1/rgstr";

// HTTP Misc
const char *kContentTypeJson = "application/json";
const char *kDefaultApi = "https://statsigapi.net";
const int kInitializeRetryCount = 3;
const int kLogEventRetryCount = 3;

// Caching
const char *kCacheDirectory = "/tmp/statsig_cpp_client";
const char *kCachedEvaluationsPrefix = "statsig.cached.evaluations.";
const char *kStableIdKey = "stable_id";
const uint kMaxCacheEntriesCount = 10;

}