#include "RandomAccessState.h"

#include "StarTopologyEmulator/Messages/StarStationMessage.h"
#include "StarTopologyEmulator/TerminalState.h"

namespace starTopologyEmulator
{

RandomAccessState::RandomAccessState(std::shared_ptr<StationContext> context)
	: State(static_cast<IState::Index>(TerminalState::ACQUISITION))
	, _context(context)
{}

void RandomAccessState::onUpdate()
{
	auto moment = _context->frameCalculator->frameMoment(_context->now);

	if (moment.frameNumber == _lastProcessedFrame && moment.slotNumber == _lastProcessedSlot)
		return;

	auto plan = _context->dynamicFrameSettings->currentPlan(moment.frameNumber);

	if (!plan)
		return;

	_lastProcessedFrame = moment.frameNumber;
	_lastProcessedSlot = moment.slotNumber;

	if (_context->now - _context->lastSendTime >= _context->ackTimeout && _context->transmitStatus != StationContext::TryingToSend)
	{
		if (_context->transmitStatus == StationContext::WaitingForAcq)
			handleCollision(plan);

		_context->transmitStatus = StationContext::TryingToSend;
	}

	if (moment.slotNumber >= plan->randomAccessSlotsCountInFrame())
		return;

	if (!_decisionMade || moment.frameNumber != _decisionFrame)
	{
		_decisionFrame = moment.frameNumber;
		_decisionMade = true;
		planFrameAttempt(plan);
	}

	if (_chosenSlot >= 0
		&& static_cast<int>(moment.slotNumber) == _chosenSlot
		&& _context->transmitStatus == StationContext::TryingToSend)
	{
		_chosenSlot = -1;
		transmit();
	}
}

void RandomAccessState::handleCollision(std::shared_ptr<StarHubPlanMessage> plan)
{
	_context->attempts++;

	const auto& cfg = plan->backoff();

	double windowSize = cfg.baseWindow;
	if (cfg.useExponential)
	{
		windowSize = cfg.baseWindow * std::pow(cfg.exponentBase, std::min((int)_context->attempts, 10));
	}
	windowSize = std::clamp(windowSize, (double)cfg.baseWindow, (double)cfg.maxWindow);

	std::uniform_int_distribution<int> dist(1, static_cast<int>(windowSize));
	_context->backoffRemaining = dist(*_context->rng);
}

void RandomAccessState::planFrameAttempt(std::shared_ptr<StarHubPlanMessage> plan)
{
	_chosenSlot = -1;

	if (_context->transmitStatus != StationContext::TryingToSend)
		return;

	if (_context->backoffRemaining > 0)
	{
		_context->backoffRemaining -= 1;
		return;
	}

	std::uniform_real_distribution<double> coin(0.0, 1.0);
	if (coin(*_context->rng) >= plan->backoff().pTx)
		return;

	const std::uint8_t raSlots = plan->randomAccessSlotsCountInFrame();
	if (raSlots == 0)
		return;

	std::uniform_int_distribution<int> slotDist(0, static_cast<int>(raSlots) - 1);
	_chosenSlot = slotDist(*_context->rng);
}

void RandomAccessState::transmit()
{
	_context->sendFunc(_context->now, std::make_shared<StarStationMessage>(_context->id));
	_context->lastSendTime = _context->now;
	_context->transmitStatus = StationContext::WaitingForAcq;
}

} // namespace starTopologyEmulator
