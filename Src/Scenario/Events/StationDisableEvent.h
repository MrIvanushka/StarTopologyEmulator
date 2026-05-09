#pragma once

#include "StarTopologyEmulator/CommonTypedefs.h"
#include "StarTopologyEmulator/IFaces/IScenarioEvent.h"

namespace starTopologyEmulator
{

class StationDisableEvent : public IScenarioEvent
{
public:
	StationDisableEvent(Timestamp at, StationID minId, StationID maxId);

	Timestamp at() const override;
	ScenarioEventKind kind() const override;
	void apply(IEmulator& emulator) const override;

private:
	Timestamp _at;
	StationID _minId;
	StationID _maxId;
};

} // namespace starTopologyEmulator
