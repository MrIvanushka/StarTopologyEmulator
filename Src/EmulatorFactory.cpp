#include "StarTopologyEmulator/EmulatorFactory.h"

#include "Emulator.h"

namespace starTopologyEmulator
{

std::unique_ptr<IEmulator> EmulatorFactory::make(EmulatorInitData initData)
{
    return std::make_unique<Emulator>(
        initData.stationFactory, 
        initData.hubFactory, 
        initData.stationCount, 
        initData.rttSlots);
}

} // namespace starTopologyEmulator
