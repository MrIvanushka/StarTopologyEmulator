#pragma once

#include <memory>

#include "StarTopologyEmulator/IFaces/IStarHubStrategy.h"
#include "StarTopologyEmulator/StarHubStrategyConfig.h"

namespace starTopologyEmulator
{

class STAR_TOPOLOGY_EMULATOR_LIB_EXPORT StarHubStrategyFactory
{
public:
    static std::unique_ptr<IStarHubStrategy> make(const StarHubStrategyConfig&&);
};

} // namespace starTopologyEmulator
