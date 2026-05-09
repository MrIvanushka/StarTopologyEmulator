#include "StationSetProfileEvent.h"

#include "StarTopologyEmulator/IFaces/IEmulator.h"
#include "StarTopologyEmulator/IFaces/IStarStation.h"

namespace starTopologyEmulator
{

StationSetProfileEvent::StationSetProfileEvent(
	Timestamp at, StationID minId, StationID maxId, ProfileBuilder builder)
	: _at(at)
	, _minId(minId)
	, _maxId(maxId)
	, _builder(std::move(builder))
{}

Timestamp StationSetProfileEvent::at() const
{
	return _at;
}

ScenarioEventKind StationSetProfileEvent::kind() const
{
	return ScenarioEventKind::STATION_SET_PROFILE;
}

void StationSetProfileEvent::apply(IEmulator& emulator) const
{
	for (const auto& station : emulator.stations())
	{
		const auto id = station->id();
		if (id >= _minId && id <= _maxId)
			station->setTrafficProfile(_builder());
	}
}

} // namespace starTopologyEmulator
