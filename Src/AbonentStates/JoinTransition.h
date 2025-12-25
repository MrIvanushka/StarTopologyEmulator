#pragma once

#include <memory>

#include "StarTopologyEmulator/StateMachine/IFaces/ITransition.h"
#include "Stations/StationContext.h"

namespace starTopologyEmulator
{

class JoinTransition : public ITransition
{
public:
    JoinTransition(
        std::shared_ptr<StationContext> context,
        IState* nextState);

    bool needTransit() override;

    IState* nextState() override;

private:
    std::shared_ptr<StationContext> _context;
    IState* _nextState;
};


} // namespace starTopologyEmulator
