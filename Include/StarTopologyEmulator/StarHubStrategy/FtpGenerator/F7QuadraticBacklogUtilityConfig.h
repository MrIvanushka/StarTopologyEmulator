#pragma once

#include <cstdint>

#include "StarTopologyEmulator/StarTopologyEmulatorExport.h"

namespace starTopologyEmulator
{

// DA: U^DA(d) = wB * ln(1 + d) - wS * (Q^DA)^2
// RA: companion F2 — U^RA(r) = wAcq*ln(1+mu^RA) - wColl*C^RA
// Quadratic backlog penalty; equivalent to Lyapunov drift-plus-penalty
// (§2.3.3.3) at V = wS / wB.
struct STAR_TOPOLOGY_EMULATOR_LIB_EXPORT F7QuadraticBacklogUtilityConfig
{
    double wAcq  = 1.0;
    double wColl = 0.5;
    double wB    = 1.0;
    double wS    = 0.1;  // weight on (Q^DA)^2 penalty term

    std::uint8_t raMin       = 5;
    std::uint8_t raMax       = 50;
    std::uint8_t yellowSlots = 0;
};

} // namespace starTopologyEmulator
