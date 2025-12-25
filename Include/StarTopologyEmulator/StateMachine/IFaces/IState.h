#pragma once

namespace starTopologyEmulator
{

class IState
{
public:
	using Index = int;
	static const Index INVALID = -1;

	using NextStatePtr = IState*;

	virtual ~IState() = default;

	virtual NextStatePtr update() = 0;

	virtual Index index() const = 0;
};

} // namespace starTopologyEmulator