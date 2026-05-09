#include "StationEnableEvent.h"

#include "StarTopologyEmulator/IFaces/IEmulator.h"
#include "StarTopologyEmulator/IFaces/IStarStation.h"

namespace starTopologyEmulator
{

StationEnableEvent::StationEnableEvent(Timestamp at, StationID minId, StationID maxId)
	: _at(at)
	, _minId(minId)
	, _maxId(maxId)
{}

Timestamp StationEnableEvent::at() const
{
	return _at;
}

ScenarioEventKind StationEnableEvent::kind() const
{
	return ScenarioEventKind::STATION_ENABLE;
}

void StationEnableEvent::apply(IEmulator& emulator) const
{
	for (const auto& station : emulator.stations())
	{
		const auto id = station->id();
		if (id >= _minId && id <= _maxId)
			station->setEnabled(true);
	}
}

} // namespace starTopologyEmulator
