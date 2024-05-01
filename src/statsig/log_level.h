#pragma once

#ifdef STATSIG_UNREAL_PLUGIN
#include "Logging/LogMacros.h"
DECLARE_LOG_CATEGORY_EXTERN(LogStatsig, Log, All);
inline DEFINE_LOG_CATEGORY(LogStatsig);
#endif

namespace statsig {

enum LogLevel: int {
  None = 0,
  Error,
  Warn,
  Debug
};

}