#pragma once

#include <cstdint>

#include "StarTopologyEmulator/StarTopologyEmulatorExport.h"

namespace starTopologyEmulator
{

// U(x) = w / (1 + exp(-k*(x - x0)))
// Non-concave; models minimum resource threshold before useful work begins.
// Greedy NUM is not optimal here — exhaustive scan is used instead.
struct STAR_TOPOLOGY_EMULATOR_LIB_EXPORT F4SigmoidalUtilityConfig
{
    double wAcq  = 1.0;
    double wAuth = 1.0;
    double wColl = 0.5;
    double wB    = 1.0;
    double wD    = 0.5;
    double k     = 1.0;   // steepness
    double x0    = 0.5;   // inflection point

    std::uint8_t raMin       = 5;
    std::uint8_t raMax       = 50;
    std::uint8_t yellowSlots = 0;
};

} // namespace starTopologyEmulator
