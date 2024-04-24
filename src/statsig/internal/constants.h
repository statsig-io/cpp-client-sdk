#pragma once

namespace statsig::constants {

// Statsig Metadata
const char *kSdkVersion = "0.0.2";

// HTTP Endpoints
const char *kEndpointInitialize = "/v1/initialize";
const char *kEndpointLogEvent = "/v1/rgstr";

// HTTP Misc
const char *kContentTypeJson = "application/json";
const char *kDefaultApi = "https://statsigapi.net";
const int kInitializeRetryCount = 3;
const int kLogEventRetryCount = 2;

// Caching
const char *kCachedEvaluationsPrefix = "statsig.cached.evaluations.";
const char *kStableIdKey = "stable_id";
const int kMaxCachedEvaluationsCount = 10;
const char *kCacheDirectory = "/tmp/statsig_cpp_client";

// Logging Cache
const char *kCachedFailedEventPayloadPrefix = "statsig.failed_events.";
const int kFailedEventPayloadRetryCount = 3;
const int kMaxCachedFailedEventPayloadsCount = 5;

// Error Reporting
const std::string kBadNetworkErr = "NetworkError_";

}