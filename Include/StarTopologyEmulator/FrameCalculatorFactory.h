#pragma once

#include <memory>

#include "StarTopologyEmulator/IFaces/IFrameCalculator.h"
#include "StarTopologyEmulator/StarTopologyEmulatorExport.h"

namespace starTopologyEmulator
{

class STAR_TOPOLOGY_EMULATOR_LIB_EXPORT FrameCalculatorFactory
{
public:
    static std::unique_ptr<IFrameCalculator> abonentCalculator(FrameConfig, Timestamp tts);

    static std::unique_ptr<IFrameCalculator> hubCalculator(FrameConfig, Timestamp tts);
};

} // namespace starTopologyEmulator
