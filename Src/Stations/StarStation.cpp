#include "StarStation.h"

#include "StarTopologyEmulator/Messages/OperationPlanMessage.h"
#include "StarTopologyEmulator/Messages/StarHubAccessMessage.h"
#include "AbonentStates/JoinTransition.h"
#include "AbonentStates/SwitchEnabledTransition.h"
#include "AbonentStates/OffState.h"
#include "AbonentStates/OperationState.h"
#include "AbonentStates/RandomAccessState.h"

namespace starTopologyEmulator
{

StarStation::StarStation(
	std::function<void(Timestamp, std::shared_ptr<IMessage>)> sendFunc,
	std::unique_ptr<IFrameCalculator> frameCalculator,
	std::unique_ptr<IDynamicFrameSettings> dynamicFrameSettings,
	std::unique_ptr<ITrafficProfile> trafficProfile,
	StationID id,
	int messagesNeeded,
	Timestamp tts,
	std::mt19937& rng,
	std::shared_ptr<IStationStatsCollector> stats)
	: _tts(tts)
	, _rng(rng)
{
	_context = std::make_shared<StationContext>();
	_context->id = id;
	_context->frameCalculator = std::move(frameCalculator);
	_context->dynamicFrameSettings = std::move(dynamicFrameSettings);
	_context->trafficProfile = std::move(trafficProfile);
	_context->messagesNeeded = messagesNeeded;
	const auto& frameConfig = _context->frameCalculator->frameConfig();
	_context->ackTimeout = 5 * tts;
	_context->rng = &_rng;
	_context->sendFunc = std::move(sendFunc);
	_context->stats = std::move(stats);

	buildStateMachine();
}

void StarStation::setTrafficProfile(std::unique_ptr<ITrafficProfile> profile)
{
	_context->trafficProfile = std::move(profile);
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
		_context->dynamicFrameSettings->handlePlan(plan);
	}
	else if (msg->type() == MessageType::OperationPlan)
	{
		auto plan = std::static_pointer_cast<OperationPlanMessage>(msg);
		_context->dynamicFrameSettings->handleOperationPlan(plan);
	}
	else if (msg->type() == MessageType::StarHubAccess)
	{
		auto ack = std::static_pointer_cast<StarHubAccessMessage>(msg);
		if (ack->stationID() == _context->id && _context->transmitStatus == StationContext::WaitingForAcq)
		{
			_context->transmitStatus = StationContext::ReceivedAcq;
			_context->backoffRemaining = 0;
			_context->attempts = 0;
			++_context->messagesDelivered;
		}
	}
}

void StarStation::setEnabled(bool value)
{
	_context->enabled = value;
}

void StarStation::buildStateMachine()
{
	State::Transitions emptyTransitions;

	auto offState = std::make_shared<OffState>();
	auto raState = std::make_shared<RandomAccessState>(_context);
	auto operationState = std::make_shared<OperationState>(_context);

	offState->setupTransitions({
		std::make_shared<SwitchEnabledTransition>(_context, raState.get(), true)
	});
	raState->setupTransitions({
		std::make_shared<SwitchEnabledTransition>(_context, offState.get(), false),
		std::make_shared<JoinTransition>(_context, operationState.get())
	});
	operationState->setupTransitions({
		std::make_shared<SwitchEnabledTransition>(_context, offState.get(), false)
	});

	std::vector<std::shared_ptr<IState>> allStates = { offState, raState, operationState };
	_stateMachine = std::make_unique<StateMachine>(allStates);
}

Timestamp StarStation::tts() const
{
	return _tts;
}

TerminalState StarStation::currentState() const
{
	return static_cast<TerminalState>(_stateMachine->currentStateIndex());
}

std::optional<Timestamp> StarStation::joinedTime() const
{
	return _context->joinedTime
		? std::optional<int>(static_cast<int>(*_context->joinedTime))
		: std::optional<int>{};
}

std::uint32_t StarStation::id() const
{
	return _context->id;
}

} // namespace starTopologyEmulator
