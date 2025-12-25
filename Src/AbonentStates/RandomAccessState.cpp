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
	if (_context->messagesDelivered >= _context->messagesNeeded)
		return;

	if (_context->waitingForAck)
	{
		if (_context->now - _context->lastSendTime >= _context->ackTimeout)
		{
			_context->waitingForAck = false;
			++_context->attempts;

			auto window = computeWindow();

			if (window > 0)
			{
				std::uniform_int_distribution<int> dist(0, window);
				_context->backoffRemaining = dist(*_context->rng);
			}
			else
			{
				_context->backoffRemaining = 0;
			}
		}
		return;
	}
	if (_context->backoffRemaining > 0)
	{
		_context->backoffRemaining -= 1;
		return;
	}
	if (!_context->plan)
	{
		return;
	}

	auto backoff = _context->plan->backoff();

	if (backoff.pTx < 1.0)
	{
		std::uniform_real_distribution<double> dist(0.0, 1.0);
		if (dist(*_context->rng) > backoff.pTx)
		return;
	}

	auto msg = std::make_shared<StarStationMessage>(_context->id);
	_context->sendFunc(_context->now, msg);
	_context->waitingForAck = true;
	_context->lastSendTime = _context->now;
}

int RandomAccessState::computeWindow() const
{
	auto cfg = _context->plan->backoff();

	if (_context->attempts <= 0)
		return 0;

	int window = 0;

	if (cfg.useExponential)
		window = static_cast<int>(cfg.baseWindow * std::pow(cfg.exponentBase, _context->attempts - 1));
	else
		window = cfg.baseWindow + (_context->attempts - 1);

	if (window < 0)
		window = 0;
	if (window > cfg.maxWindow)
		window = cfg.maxWindow;

	return window;
}

} // namespace starTopologyEmulator
