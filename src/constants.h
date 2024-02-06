#pragma once

namespace statsig::constants {

// Statsig Metadata
inline constexpr const char *kSdkType = "cpp-client";
inline constexpr const char *kSdkVersion = "0.0.1";

// HTTP Headers
inline constexpr const char *kHeaderSdkType = "STATSIG-SDK-TYPE";
inline constexpr const char *kHeaderSdkVersion = "STATSIG-SDK-VERSION";
inline constexpr const char *kHeaderServerSessionId = "STATSIG-SERVER-SESSION-ID";
inline constexpr const char *kHeaderClientTime = "STATSIG-CLIENT-TIME";
inline constexpr const char *kHeaderApiKey = "STATSIG-API-KEY";

// HTTP Endpoints
inline constexpr const char *kEndpointInitialize = "/initialize";

// HTTP Misc
inline constexpr const char *kContentTypeJson = "application/json";

}