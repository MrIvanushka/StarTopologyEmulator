#include "StarTopologyEmulator/Scenario/ScenarioEventFactory.h"

#include "Events/StationDisableEvent.h"
#include "Events/StationEnableEvent.h"
#include "Events/StationSetProfileEvent.h"

namespace starTopologyEmulator
{

std::unique_ptr<IScenarioEvent>
ScenarioEventFactory::makeStationEnable(Timestamp at, StationID minId, StationID maxId)
{
	return std::make_unique<StationEnableEvent>(at, minId, maxId);
}

std::unique_ptr<IScenarioEvent>
ScenarioEventFactory::makeStationDisable(Timestamp at, StationID minId, StationID maxId)
{
	return std::make_unique<StationDisableEvent>(at, minId, maxId);
}

std::unique_ptr<IScenarioEvent>
ScenarioEventFactory::makeStationSetProfile(
	Timestamp at, StationID minId, StationID maxId, ProfileBuilder builder)
{
	return std::make_unique<StationSetProfileEvent>(at, minId, maxId, std::move(builder));
}

} // namespace starTopologyEmulator
