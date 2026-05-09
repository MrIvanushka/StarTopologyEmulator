#pragma once

#include <functional>
#include <memory>

#include "StarTopologyEmulator/CommonTypedefs.h"
#include "StarTopologyEmulator/IFaces/IScenarioEvent.h"
#include "StarTopologyEmulator/IFaces/ITrafficProfile.h"

namespace starTopologyEmulator
{

class StationSetProfileEvent : public IScenarioEvent
{
public:
	using ProfileBuilder = std::function<std::unique_ptr<ITrafficProfile>()>;

	StationSetProfileEvent(Timestamp at, StationID minId, StationID maxId, ProfileBuilder builder);

	Timestamp at() const override;
	ScenarioEventKind kind() const override;
	void apply(IEmulator& emulator) const override;

private:
	Timestamp _at;
	StationID _minId;
	StationID _maxId;
	ProfileBuilder _builder;
};

} // namespace starTopologyEmulator
