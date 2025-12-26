#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "StarTopologyEmulator/IFaces/IStarHubStrategy.h"

namespace starTopologyEmulator
{

class STAR_TOPOLOGY_EMULATOR_LIB_EXPORT StarHubStrategyFactory
{
public:
    static std::unique_ptr<IStarHubStrategy> make();
};

} // namespace starTopologyEmulator
