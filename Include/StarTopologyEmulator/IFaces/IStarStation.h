#pragma once

#include <cstdint>
#include <memory>
#include <optional>

#include "StarTopologyEmulator/CommonTypedefs.h"
#include "StarTopologyEmulator/IFaces/ITerminal.h"
#include "StarTopologyEmulator/IFaces/ITrafficProfile.h"
#include "StarTopologyEmulator/TerminalState.h"

namespace starTopologyEmulator
{

class IStarStation : public ITerminal
{
public:
	virtual TerminalState currentState() const = 0;

	virtual std::optional<Timestamp> joinedTime() const = 0;

	virtual StationID id() const = 0;

	virtual void setTrafficProfile(std::unique_ptr<ITrafficProfile>) = 0;
};

} // namespace starTopologyEmulator