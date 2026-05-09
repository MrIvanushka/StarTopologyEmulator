#pragma once

#include "StarTopologyEmulator/CommonTypedefs.h"
#include "StarTopologyEmulator/StarTopologyEmulatorExport.h"

namespace starTopologyEmulator
{

class IEmulator;

enum class STAR_TOPOLOGY_EMULATOR_LIB_EXPORT ScenarioEventKind : int
{
	STATION_ENABLE = 0,
	STATION_DISABLE,
	STATION_SET_PROFILE,
	CAPACITY_REQUEST_SET
};

class IScenarioEvent
{
public:
	virtual ~IScenarioEvent() = default;

	// Время срабатывания на той же шкале, что IEmulator::update.
	virtual Timestamp at() const = 0;

	virtual ScenarioEventKind kind() const = 0;

	virtual void apply(IEmulator& emulator) const = 0;
};

} // namespace starTopologyEmulator
