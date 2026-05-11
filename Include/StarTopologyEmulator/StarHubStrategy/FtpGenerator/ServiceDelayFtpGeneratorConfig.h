#pragma once

#include <cstdint>

#include "StarTopologyEmulator/StarTopologyEmulatorExport.h"

namespace starTopologyEmulator
{

struct STAR_TOPOLOGY_EMULATOR_LIB_EXPORT ServiceDelayFtpGeneratorConfig
{
    double d0 = 0.0;
    double lambdaRatio = 1.0;
    std::uint8_t raMin = 5;
    std::uint8_t raMax = 50;
    std::uint8_t yellowSlots = 0;
};

} // namespace starTopologyEmulator
