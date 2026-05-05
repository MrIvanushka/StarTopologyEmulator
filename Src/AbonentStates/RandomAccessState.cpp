#include "RandomAccessState.h"

#include "StarTopologyEmulator/Messages/StarStationMessage.h"
#include "StarTopologyEmulator/TerminalState.h"

namespace starTopologyEmulator
{

RandomAccessState::RandomAccessState(
	std::shared_ptr<StationContext> context,
	const Transitions& transitions)
	: State(transitions, static_cast<IState::Index>(TerminalState::ACQUISITION))
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
	{
		return;
	}
	implementBackoffLogic(plan);
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

void RandomAccessState::implementBackoffLogic(std::shared_ptr<StarHubPlanMessage> plan)
{
	if (_context->backoffRemaining > 0)
	{
		_context->backoffRemaining -= 1;
		return;
	}
	std::uniform_real_distribution<double> dist(0.0, 1.0);

	if (dist(*_context->rng) > plan->backoff().pTx)
		return;

	transmit();
}

void RandomAccessState::transmit()
{
	_context->sendFunc(_context->now, std::make_shared<StarStationMessage>(_context->id));
	_context->lastSendTime = _context->now;
	_context->transmitStatus = StationContext::WaitingForAcq;
}

} // namespace starTopologyEmulator
