#pragma once

#include <cstdint>

#include <gmock/gmock.h>

#include "StarTopologyEmulator/IFaces/IFtpGenerator.h"
#include "StarTopologyEmulator/Messages/StarHubPlanMessage.h"

namespace tests
{

class MockFtpGenerator : public starTopologyEmulator::IFtpGenerator
{
public:
	MOCK_METHOD(starTopologyEmulator::StarHubPlanMessage::FtpConfig, generate,
		(std::uint64_t), (override));
};

} // namespace tests
