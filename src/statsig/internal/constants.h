#pragma once

namespace statsig::constants {

// Statsig Metadata
inline const char *kSdkVersion = "0.0.5";

// HTTP Endpoints
inline const char *kEndpointInitialize = "/v1/initialize";
inline const char *kEndpointLogEvent = "/v1/rgstr";

// HTTP Misc
inline const char *kContentTypeJson = "application/json";
inline const char *kDefaultApi = "https://statsigapi.net";
inline const int kInitializeRetryCount = 3;
inline const int kLogEventRetryCount = 2;

// Caching
inline const char *kCachedEvaluationsPrefix = "statsig.cached.evaluations.";
inline const char *kStableIdKey = "stable_id";
inline const int kMaxCachedEvaluationsCount = 10;
inline const char *kCacheDirectory = "/tmp/statsig_cpp_client";

// Logging
inline const char *kCachedFailedEventPayloadPrefix = "statsig.failed_events.";
inline const int kFailedEventPayloadRetryCount = 3;
inline const int kMaxCachedFailedEventPayloadsCount = 5;
inline const int kLoggingIntervalMs = 10000;
inline const int kMaxQueuedEvents = 50;
inline const int kMaxDiagnosticsMarkers = 30;

// Error Reporting
const std::string kBadNetworkErr = "NetworkError_";
const std::string kShutdownTimeoutExtra = "ShutdownTimeout";
inline const int kShutdownTimeoutErrorThresholdMs = 2000;

}