#pragma once

#include <gmock/gmock.h>

#include "StarTopologyEmulator/CommonTypedefs.h"
#include "StarTopologyEmulator/IFaces/IScenarioEvent.h"

namespace tests
{

class MockScenarioEvent : public starTopologyEmulator::IScenarioEvent
{
public:
	MOCK_METHOD(starTopologyEmulator::Timestamp, at, (), (const, override));
	MOCK_METHOD(starTopologyEmulator::ScenarioEventKind, kind, (), (const, override));
	MOCK_METHOD(void, apply, (starTopologyEmulator::IEmulator&), (const, override));
};

} // namespace tests
