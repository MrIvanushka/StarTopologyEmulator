#include "StarStation.h"

#include "StarTopologyEmulator/Messages/StarHubAccessMessage.h"
#include "AbonentStates/JoinTransition.h"
#include "AbonentStates/OffState.h"
#include "AbonentStates/OperationState.h"
#include "AbonentStates/RandomAccessState.h"

namespace starTopologyEmulator
{

StarStation::StarStation(
	std::function<void(Timestamp, std::shared_ptr<IMessage>)> sendFunc,
	StationID id,
	int messagesNeeded,
	int rttSlots,
	std::mt19937& rng)
	: _rttSlots(rttSlots)
	, _rng(rng)
{
	_context = std::make_shared<StationContext>();

	_context->id = id;
	_context->messagesNeeded = messagesNeeded;
	_context->ackTimeout = 2 * rttSlots;
	_context->rng = &_rng;
	_context->sendFunc = std::move(sendFunc);

	buildStateMachine();
}

void StarStation::update(Timestamp currentTime)
{
	_context->now = currentTime;
	if (_stateMachine)
	_stateMachine->update();
}

void StarStation::handleMessage(std::shared_ptr<IMessage> msg, Timestamp timestamp)
{
	_context->now = timestamp;

	if (msg->type() == MessageType::StarHubPlan)
	{
		auto plan = std::static_pointer_cast<StarHubPlanMessage>(msg);
		_context->plan = plan;
	}
	else if (msg->type() == MessageType::StarHubAccess)
	{
		auto ack = std::static_pointer_cast<StarHubAccessMessage>(msg);
		if (ack->stationID() == _context->id && _context->waitingForAck)
		{
			_context->waitingForAck = false;
			_context->backoffRemaining = 0;
			_context->attempts = 0;
			++_context->messagesDelivered;
		}
	}
}

TerminalState StarStation::currentState() const
{
	return static_cast<TerminalState>(_stateMachine->currentStateIndex());
}

std::optional<StarStation::Timestamp> StarStation::joinedTime() const
{
	return _context->joinedTime
		? std::optional<int>(static_cast<int>(*_context->joinedTime))
		: std::optional<int>{};
}

std::uint32_t StarStation::id() const
{
	return _context->id;
}

void StarStation::buildStateMachine()
{
	State::Transitions emptyTransitions;

	auto joinedState = std::make_shared<OperationState>(emptyTransitions);

	State::Transitions raTransitions;
	raTransitions.push_back(
	std::make_shared<JoinTransition>(_context, joinedState.get()));

	auto raState = std::make_shared<RandomAccessState>(_context, raTransitions);

	std::vector<std::shared_ptr<IState>> allStates;
	allStates.push_back(raState);
	allStates.push_back(joinedState);

	_stateMachine = std::make_unique<StateMachine>(allStates);
}

} // namespace starTopologyEmulator
