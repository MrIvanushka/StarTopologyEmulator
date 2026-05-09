#pragma once

#include <memory>
#include <vector>

#include <gmock/gmock.h>

#include "StarTopologyEmulator/IFaces/IEmulator.h"
#include "StarTopologyEmulator/IFaces/IStarHub.h"
#include "StarTopologyEmulator/IFaces/IStarStation.h"
#include "StarTopologyEmulator/Metrics/MetricSink.h"

namespace tests
{

class MockEmulator : public starTopologyEmulator::IEmulator
{
public:
	MOCK_METHOD(void, update, (starTopologyEmulator::IEmulator::Timestamp), (override));
	MOCK_METHOD(std::shared_ptr<starTopologyEmulator::IStarHub>, hub, (), (const, override));
	MOCK_METHOD(
		(const std::vector<std::shared_ptr<starTopologyEmulator::IStarStation>>&),
		stations, (), (const, override));
	MOCK_METHOD(
		std::shared_ptr<starTopologyEmulator::IMetricSink>, metricSink, (), (const, override));
};

} // namespace tests
