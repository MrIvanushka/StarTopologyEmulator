#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <random>

#include "StarTopologyEmulator/CommonTypedefs.h"
#include "StarTopologyEmulator/IFaces/ITerminal.h"
#include "StarTopologyEmulator/Messages/StarHubPlanMessage.h"

namespace starTopologyEmulator
{

struct StationContext
{
    StationID id = 0;

    int messagesNeeded = 0;
    int messagesDelivered = 0;
    int attempts = 0;
    int backoffRemaining = 0;

    std::shared_ptr<StarHubPlanMessage> plan;

    bool waitingForAck = false;
    ITerminal::Timestamp lastSendTime = 0;
    ITerminal::Timestamp ackTimeout = 0;  // в слотах

    ITerminal::Timestamp now = 0;

    std::mt19937* rng = nullptr;
    std::function<void(ITerminal::Timestamp, std::shared_ptr<IMessage>)> sendFunc;

    std::optional<ITerminal::Timestamp> joinedTime;
};

} // namespace starTopologyEmulator
