#pragma once

#include <memory>

#include <gmock/gmock.h>

#include "StarTopologyEmulator/CommonTypedefs.h"
#include "StarTopologyEmulator/IFaces/IMessage.h"
#include "StarTopologyEmulator/IFaces/IStarHub.h"

namespace tests
{

class MockStarHub : public starTopologyEmulator::IStarHub
{
public:
	MOCK_METHOD(void, update, (starTopologyEmulator::Timestamp), (override));
	MOCK_METHOD(void, handleMessage,
		(std::shared_ptr<starTopologyEmulator::IMessage>, starTopologyEmulator::Timestamp),
		(override));
	MOCK_METHOD(starTopologyEmulator::Timestamp, tts, (), (const, override));
};

} // namespace tests
