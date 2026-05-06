#include "OperationState.h"

#include <algorithm>

#include "StarTopologyEmulator/Messages/BacklogReportMessage.h"
#include "StarTopologyEmulator/TerminalState.h"

namespace starTopologyEmulator
{

OperationState::OperationState(
	std::shared_ptr<StationContext> context,
	const Transitions& transitions)
	: State(transitions, static_cast<Index>(TerminalState::OPERATION))
	, _context(std::move(context))
{
}

void OperationState::onUpdate()
{
	auto moment = _context->frameCalculator->frameMoment(_context->now);
	if (moment.frameNumber <= _lastProcessedFrame)
		return;

	const auto& fc = _context->frameCalculator->frameConfig();
	const Timestamp frameDuration =
		static_cast<Timestamp>(fc.slotCountInFrame) * fc.slotDuration;

	_context->backlogBits += _context->trafficProfile->generateBits(frameDuration);

	auto plan = _context->dynamicFrameSettings->currentOperationPlan(moment.frameNumber);
	if (plan)
	{
		const auto allocation = plan->allocationFor(_context->id);
		const auto consumed = std::min(allocation, _context->backlogBits);
		_context->backlogBits -= consumed;
	}

	_context->sendFunc(
		_context->now,
		std::make_shared<BacklogReportMessage>(_context->id, _context->backlogBits));

	_lastProcessedFrame = moment.frameNumber;
}

} // namespace starTopologyEmulator
