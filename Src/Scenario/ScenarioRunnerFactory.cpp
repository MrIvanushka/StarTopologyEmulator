#include "StarTopologyEmulator/Scenario/ScenarioRunnerFactory.h"

#include "ScenarioRunner.h"

namespace starTopologyEmulator
{

std::unique_ptr<IScenarioRunner> ScenarioRunnerFactory::make(std::shared_ptr<IEmulator> emulator)
{
	return std::make_unique<ScenarioRunner>(std::move(emulator));
}

} // namespace starTopologyEmulator
