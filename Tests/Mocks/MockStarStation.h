#pragma once

#include <memory>
#include <optional>

#include <gmock/gmock.h>

#include "StarTopologyEmulator/CommonTypedefs.h"
#include "StarTopologyEmulator/IFaces/IMessage.h"
#include "StarTopologyEmulator/IFaces/IStarStation.h"
#include "StarTopologyEmulator/TerminalState.h"

namespace tests
{

class MockStarStation : public starTopologyEmulator::IStarStation
{
public:
	MOCK_METHOD(void, update, (starTopologyEmulator::Timestamp), (override));
	MOCK_METHOD(void, handleMessage,
		(std::shared_ptr<starTopologyEmulator::IMessage>, starTopologyEmulator::Timestamp),
		(override));
	MOCK_METHOD(starTopologyEmulator::Timestamp, tts, (), (const, override));
	MOCK_METHOD(starTopologyEmulator::TerminalState, currentState, (), (const, override));
	MOCK_METHOD((std::optional<starTopologyEmulator::Timestamp>), joinedTime, (), (const, override));
	MOCK_METHOD(starTopologyEmulator::StationID, id, (), (const, override));
};

} // namespace tests
