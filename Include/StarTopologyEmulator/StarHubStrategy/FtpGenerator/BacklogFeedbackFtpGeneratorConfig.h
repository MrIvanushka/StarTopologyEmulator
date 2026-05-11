#pragma once

#include <cstdint>

#include "StarTopologyEmulator/StarTopologyEmulatorExport.h"

namespace starTopologyEmulator
{

struct STAR_TOPOLOGY_EMULATOR_LIB_EXPORT BacklogFeedbackFtpGeneratorConfig
{
    double rhoAuth = 2.0;
    double R0 = 20.0;
    double kJ = 1.0;
    double kQ = 0.1;
    double jStar = 10.0;
    double qStar = 100.0;
    double deltaR = 5.0;
    std::uint8_t raMin = 5;
    std::uint8_t raMax = 50;
    std::uint8_t yellowSlots = 0;
};

} // namespace starTopologyEmulator
