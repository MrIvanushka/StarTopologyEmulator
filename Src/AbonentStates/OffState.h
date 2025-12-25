#pragma once

#include "StarTopologyEmulator/StateMachine/State.h"

namespace starTopologyEmulator
{

class OffState final : public State
{
public:
    OffState(const Transitions&);

private:
    void onUpdate() override {}
};

} // namespace starTopologyEmulator
