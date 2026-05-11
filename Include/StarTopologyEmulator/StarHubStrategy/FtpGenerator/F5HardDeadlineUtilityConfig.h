#pragma once

#include <cstdint>

#include "StarTopologyEmulator/StarTopologyEmulatorExport.h"

namespace starTopologyEmulator
{

// DA: U^DA(d) = -B * max(0, D^DA(d) - Dtar)^2 + wB * ln(d)
// RA: companion F2 — U^RA(r) = wAcq*ln(1+mu^RA) - wColl*C^RA
// For real-time traffic (VoIP, telemetry); shifts allocation sharply toward DA
// as D^DA approaches Dtar.
struct STAR_TOPOLOGY_EMULATOR_LIB_EXPORT F5HardDeadlineUtilityConfig
{
    double wAcq  = 1.0;
    double wColl = 0.5;
    double wB    = 1.0;   // coefficient for ln(d) term in DA utility
    double B     = 10.0;  // deadline penalty coefficient
    double Dtar  = 10.0;  // target delay (frames)

    std::uint8_t raMin       = 5;
    std::uint8_t raMax       = 50;
    std::uint8_t yellowSlots = 0;
};

} // namespace starTopologyEmulator
