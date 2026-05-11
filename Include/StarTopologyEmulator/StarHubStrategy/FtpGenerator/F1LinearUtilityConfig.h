#pragma once

#include <cstdint>

#include "StarTopologyEmulator/StarTopologyEmulatorExport.h"

namespace starTopologyEmulator
{

struct STAR_TOPOLOGY_EMULATOR_LIB_EXPORT F1LinearUtilityConfig
{
    double wAcq  = 1.0;
    double wAuth = 1.0;
    double wColl = 0.5;
    double wB    = 1.0;
    double wD    = 0.5;

    std::uint8_t raMin       = 5;
    std::uint8_t raMax       = 50;
    std::uint8_t yellowSlots = 0;
};

} // namespace starTopologyEmulator
