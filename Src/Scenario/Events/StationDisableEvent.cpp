#include "StationDisableEvent.h"

#include "StarTopologyEmulator/IFaces/IEmulator.h"
#include "StarTopologyEmulator/IFaces/IStarStation.h"

namespace starTopologyEmulator
{

StationDisableEvent::StationDisableEvent(Timestamp at, StationID minId, StationID maxId)
	: _at(at)
	, _minId(minId)
	, _maxId(maxId)
{}

Timestamp StationDisableEvent::at() const
{
	return _at;
}

ScenarioEventKind StationDisableEvent::kind() const
{
	return ScenarioEventKind::STATION_DISABLE;
}

void StationDisableEvent::apply(IEmulator& emulator) const
{
	for (const auto& station : emulator.stations())
	{
		const auto id = station->id();
		if (id >= _minId && id <= _maxId)
			station->setEnabled(false);
	}
}

} // namespace starTopologyEmulator
