#pragma once

#include <cstdint>

#include "StarTopologyEmulator/StarTopologyEmulatorExport.h"

namespace starTopologyEmulator
{

// cµ-rule (Buyukkoc–Varaiya–Walrand 1985):
//   U^DA(d) = -cD * D^DA(d) * Q^DA
//   U^RA(r) = -cJ * D^RA(r) * J^RA
// Optimal for priority scheduling in stationary queues.
struct STAR_TOPOLOGY_EMULATOR_LIB_EXPORT F6CostOfDelayUtilityConfig
{
    double cD = 1.0;  // cost per unit of DA delay weighted by DA backlog
    double cJ = 1.0;  // cost per unit of RA delay weighted by RA demand
    double d0 = 0.0;  // constant RTT offset in D^RA formula (frames)

    std::uint8_t raMin       = 5;
    std::uint8_t raMax       = 50;
    std::uint8_t yellowSlots = 0;
};

} // namespace starTopologyEmulator
