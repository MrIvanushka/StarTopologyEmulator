#pragma once

#include "IState.h"

namespace starTopologyEmulator
{

class ITransition
{
public:
	virtual ~ITransition() = default;

	virtual bool needTransit() = 0;

	virtual IState* nextState() = 0;
};

} // namespace starTopologyEmulator