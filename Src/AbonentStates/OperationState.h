#pragma once

#include "StarTopologyEmulator/StateMachine/State.h"

namespace starTopologyEmulator
{

class OperationState final : public State
{
public:
	OperationState(const Transitions&);

private:
	void onUpdate() override {}
};

} // namespace starTopologyEmulator
