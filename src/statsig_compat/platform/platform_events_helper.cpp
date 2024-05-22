#ifndef STATSIG_UNREAL_PLUGIN

#include "platform_events_helper.hpp"

namespace statsig_compatibility {

statsig::internal::Shareable<PlatformEventsHelper> PlatformEventsHelper::shareable_ =
    statsig::internal::Shareable<PlatformEventsHelper>();

}

#endif
