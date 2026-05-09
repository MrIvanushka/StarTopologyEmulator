#pragma once

#include <memory>

#include "StarTopologyEmulator/IFaces/IEmulator.h"
#include "StarTopologyEmulator/IFaces/IScenarioRunner.h"
#include "StarTopologyEmulator/StarTopologyEmulatorExport.h"

namespace starTopologyEmulator
{

class STAR_TOPOLOGY_EMULATOR_LIB_EXPORT ScenarioRunnerFactory
{
public:
	static std::unique_ptr<IScenarioRunner> make(std::shared_ptr<IEmulator> emulator);
};

} // namespace starTopologyEmulator
