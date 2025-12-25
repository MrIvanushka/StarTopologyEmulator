#pragma once

#include <functional>
#include <map>
#include <memory>

#include "StarTopologyEmulator/IFaces/IStarHub.h"
#include "StarTopologyEmulator/IFaces/IStarStation.h"
#include "StarTopologyEmulator/TerminalState.h"

namespace starTopologyEmulator
{

class IEmulator
{
public:
	using Timestamp = std::uint64_t;
	using StateCounter = std::map<TerminalState, std::uint64_t>;

	virtual ~IEmulator() = default;

	virtual void update(Timestamp currentTime) = 0;

	virtual std::shared_ptr<IStarHub> hub() const = 0;

	virtual const std::vector<std::shared_ptr<IStarStation>>& stations() const = 0;

	virtual const std::map<StationID, StateCounter>& stationStats() const = 0;
};

} // namespace starTopologyEmulator
