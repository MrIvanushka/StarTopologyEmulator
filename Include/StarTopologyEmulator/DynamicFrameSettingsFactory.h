#pragma once

#include <memory>

#include "StarTopologyEmulator/IFaces/IDynamicFrameSettings.h"

namespace starTopologyEmulator
{

class STAR_TOPOLOGY_EMULATOR_LIB_EXPORT DynamicFrameSettingsFactory
{
public:
    static std::unique_ptr<IDynamicFrameSettings> make(int maxPlansStored = 1'000);
};

} // namespace starTopologyEmulator
