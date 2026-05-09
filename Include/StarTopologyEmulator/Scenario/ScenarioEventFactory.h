#pragma once

#include <functional>
#include <limits>
#include <memory>

#include "StarTopologyEmulator/CommonTypedefs.h"
#include "StarTopologyEmulator/IFaces/IScenarioEvent.h"
#include "StarTopologyEmulator/IFaces/ITrafficProfile.h"
#include "StarTopologyEmulator/StarTopologyEmulatorExport.h"

namespace starTopologyEmulator
{

class STAR_TOPOLOGY_EMULATOR_LIB_EXPORT ScenarioEventFactory
{
public:
	using ProfileBuilder = std::function<std::unique_ptr<ITrafficProfile>()>;

	static std::unique_ptr<IScenarioEvent>
		makeStationEnable(Timestamp at, StationID minId, StationID maxId);

	static std::unique_ptr<IScenarioEvent>
		makeStationDisable(Timestamp at, StationID minId, StationID maxId);

	static std::unique_ptr<IScenarioEvent>
		makeStationSetProfile(Timestamp at, StationID minId, StationID maxId, ProfileBuilder builder);
};

// Sentinel для "все станции": передать в minId/maxId вместо конкретного диапазона.
inline constexpr StationID kMinStationId = 0;
inline constexpr StationID kMaxStationId = std::numeric_limits<StationID>::max();

} // namespace starTopologyEmulator
