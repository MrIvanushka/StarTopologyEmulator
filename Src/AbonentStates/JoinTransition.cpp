#include "JoinTransition.h"

namespace starTopologyEmulator
{


JoinTransition::JoinTransition(
    std::shared_ptr<StationContext> context,
    IState* nextState)
    : _context(context)
    , _nextState(nextState)
{
}

bool JoinTransition::needTransit()
{
    if (_context->messagesDelivered >= _context->messagesNeeded &&
        !_context->joinedTime.has_value())
    {
        _context->joinedTime = _context->now;
        return true;
    }
    return false;
}

IState* JoinTransition::nextState()
{
    return _nextState;
}

} // namespace starTopologyEmulator
