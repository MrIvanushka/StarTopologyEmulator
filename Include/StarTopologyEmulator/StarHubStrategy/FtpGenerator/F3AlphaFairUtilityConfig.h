#pragma once

#include <cstdint>

#include "StarTopologyEmulator/StarTopologyEmulatorExport.h"

namespace starTopologyEmulator
{

// U^(alpha)(x) = (x+eps)^(1-alpha)/(1-alpha)  for alpha!=1
//              = ln(x+eps)                      for alpha==1
// Mo–Walrand 2000: alpha=0 max-throughput, alpha=1 proportional fairness,
//                  alpha->inf max-min fairness
struct STAR_TOPOLOGY_EMULATOR_LIB_EXPORT F3AlphaFairUtilityConfig
{
    double wAcq    = 1.0;
    double wAuth   = 1.0;
    double wColl   = 0.5;
    double wB      = 1.0;
    double wD      = 0.5;
    double alphaFair = 1.0;
    double epsilon   = 1e-6;

    std::uint8_t raMin       = 5;
    std::uint8_t raMax       = 50;
    std::uint8_t yellowSlots = 0;
};

} // namespace starTopologyEmulator
