#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <random>

#include "StarTopologyEmulator/CommonTypedefs.h"
#include "StarTopologyEmulator/IFaces/IDynamicFrameSettings.h"
#include "StarTopologyEmulator/IFaces/IFrameCalculator.h"
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

	StationID id = 0;

	int messagesNeeded = 0;
	int messagesDelivered = 0;
	int attempts = 0;
	int backoffRemaining = 0;

	TransitStatus transmitStatus = TryingToSend;
	Timestamp lastSendTime = 0;
	Timestamp ackTimeout = 0;

	Timestamp now = 0;

	std::mt19937* rng = nullptr;
	std::function<void(Timestamp, std::shared_ptr<IMessage>)> sendFunc;

	std::optional<Timestamp> joinedTime;
};

} // namespace starTopologyEmulator
