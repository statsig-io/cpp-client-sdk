#pragma once

#include <string>

#include "statsig_compat/defines/module_definitions.h"

namespace statsig::constants {

// Statsig Metadata
static const char *kSdkVersion = "0.0.7";

// HTTP Endpoints
static const char *kEndpointInitialize = "/v1/initialize";
static const char *kEndpointLogEvent = "/v1/rgstr";

// HTTP Misc
static const char *kContentTypeJson = "application/json";
static const char *kDefaultApi = "https://statsigapi.net";
static const int kInitializeRetryCount = 3;
static const int kLogEventRetryCount = 2;

// Caching
static const char *kCachedEvaluationsPrefix = "statsig.cached.evaluations.";
static const char *kStableIdKey = "stable_id";
static const int kMaxCachedEvaluationsCount = 10;
static const char *kCacheDirectory = "/tmp/statsig_cpp_client";

// Logging
static const char *kCachedFailedEventPayloadPrefix = "statsig.failed_events.";
static const int kFailedEventPayloadRetryCount = 3;
static const int kMaxCachedFailedEventPayloadsCount = 5;
static const int kLoggingIntervalMs = 10000;
static const int kMaxQueuedEvents = 50;
static const int kMaxDiagnosticsMarkers = 30;

// Error Reporting
STATSIG_EXPORT extern const std::string kBadNetworkErr;
STATSIG_EXPORT extern const std::string kShutdownTimeoutExtra;
static const int kShutdownTimeoutErrorThresholdMs = 2000;

}