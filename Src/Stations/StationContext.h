#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <random>

#include "StarTopologyEmulator/CommonTypedefs.h"
#include "StarTopologyEmulator/IFaces/IDynamicFrameSettings.h"
#include "StarTopologyEmulator/IFaces/IFrameCalculator.h"
#include "StarTopologyEmulator/IFaces/IStationStatsCollector.h"
#include "StarTopologyEmulator/IFaces/ITrafficProfile.h"
#include "StarTopologyEmulator/Messages/StarHubPlanMessage.h"

namespace starTopologyEmulator
{

struct StationContext
{
	enum TransitStatus
	{
		WaitingForAcq,
		ReceivedAcq,
		TryingToSend
	};

	std::unique_ptr<IFrameCalculator> frameCalculator;
	std::unique_ptr<IDynamicFrameSettings> dynamicFrameSettings;
	std::unique_ptr<ITrafficProfile> trafficProfile;

	StationID id = 0;
	bool enabled = true;

	int messagesNeeded = 0;
	int messagesDelivered = 0;
	int attempts = 0;
	int backoffRemaining = 0;
	int currentWindow = 0;

	TransitStatus transmitStatus = TryingToSend;
	Timestamp lastSendTime = 0;
	Timestamp ackTimeout = 0;

	Timestamp now = 0;

	std::mt19937* rng = nullptr;
	std::function<void(Timestamp, std::shared_ptr<IMessage>)> sendFunc;
	std::shared_ptr<IStationStatsCollector> stats;

	std::optional<Timestamp> joinedTime;

	// Operation phase state.
	std::uint64_t backlogBits = 0;
};

} // namespace starTopologyEmulator
