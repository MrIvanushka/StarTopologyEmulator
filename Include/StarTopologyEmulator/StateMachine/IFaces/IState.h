#pragma once

#include <memory>
#include <vector>

namespace starTopologyEmulator
{

class ITransition;

class IState
{
public:
	using Index = int;
	static const Index INVALID = -1;

	using NextStatePtr = IState*;

	virtual ~IState() = default;

	virtual void setupTransitions(std::vector<std::shared_ptr<ITransition>>&&) = 0;

	virtual NextStatePtr update() = 0;

	virtual Index index() const = 0;
};

} // namespace starTopologyEmulator