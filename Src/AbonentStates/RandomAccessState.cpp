#include "RandomAccessState.h"

#include <algorithm>
#include <cmath>

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

	if (_context->messagesDelivered > _deliveredSnapshot)
	{
		handleSuccess(plan);
		_deliveredSnapshot = _context->messagesDelivered;
	}

	if (_context->now - _context->lastSendTime >= _context->ackTimeout && _context->transmitStatus != StationContext::TryingToSend)
	{
		if (_context->transmitStatus == StationContext::WaitingForAcq)
			handleCollision(plan);

		_context->transmitStatus = StationContext::TryingToSend;
	}

	if (plan->frame() != _lastObservedPlanFrame)
	{
		if (plan->backoff().backoffType == StarHubPlanMessage::BackoffType::LMILD
			&& plan->collidedStationCount() > 0
			&& !_ownCollisionSinceLastPlan)
		{
			handleObservedCollision(plan);
		}
		_lastObservedPlanFrame = plan->frame();
		_ownCollisionSinceLastPlan = false;
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
	_ownCollisionSinceLastPlan = true;

	const auto& cfg = plan->backoff();
	const int wMin = std::max<int>(1, cfg.baseWindow);
	const int wMax = std::max<int>(wMin, cfg.maxWindow);

	int windowSize = wMin;
	switch (cfg.backoffType)
	{
	case StarHubPlanMessage::BackoffType::NONE:
		windowSize = wMin;
		break;
	case StarHubPlanMessage::BackoffType::BEB:
	{
		const double w = cfg.baseWindow * std::pow(cfg.exponentBase, std::min(_context->attempts, 10));
		windowSize = static_cast<int>(std::clamp(w, static_cast<double>(wMin), static_cast<double>(wMax)));
		break;
	}
	case StarHubPlanMessage::BackoffType::MILD:
	case StarHubPlanMessage::BackoffType::LMILD:
	{
		const int prev = _context->currentWindow > 0 ? _context->currentWindow : wMin;
		const double w = prev * cfg.exponentBase;
		windowSize = std::min<int>(static_cast<int>(w), wMax);
		windowSize = std::max(windowSize, wMin);
		_context->currentWindow = windowSize;
		break;
	}
	}

	std::uniform_int_distribution<int> dist(1, windowSize);
	_context->backoffRemaining = dist(*_context->rng);
}

void RandomAccessState::handleObservedCollision(std::shared_ptr<StarHubPlanMessage> plan)
{
	const auto& cfg = plan->backoff();
	const int wMin = std::max<int>(1, cfg.baseWindow);
	const int wMax = std::max<int>(wMin, cfg.maxWindow);
	const int delta = std::max<int>(1, cfg.additiveStep);

	const int prev = _context->currentWindow > 0 ? _context->currentWindow : wMin;
	_context->currentWindow = std::min(prev + delta, wMax);
}

void RandomAccessState::handleSuccess(std::shared_ptr<StarHubPlanMessage> plan)
{
	const auto& cfg = plan->backoff();
	const int wMin = std::max<int>(1, cfg.baseWindow);

	if (cfg.backoffType == StarHubPlanMessage::BackoffType::MILD)
	{
		const int prev = _context->currentWindow > 0 ? _context->currentWindow : wMin;
		_context->currentWindow = std::max(prev - 1, wMin);
	}
	else if (cfg.backoffType == StarHubPlanMessage::BackoffType::LMILD)
	{
		const int prev = _context->currentWindow > 0 ? _context->currentWindow : wMin;
		const int delta = std::max<int>(1, cfg.additiveStep);
		_context->currentWindow = std::max(prev - delta, wMin);
	}

	_context->attempts = 0;
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

	if (_context->stats)
		_context->stats->onPtxCheck(_decisionFrame);

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
