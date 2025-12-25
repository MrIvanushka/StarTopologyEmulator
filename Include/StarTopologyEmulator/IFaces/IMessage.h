#pragma once

namespace starTopologyEmulator
{

enum class MessageType
{
	StarHubAccess,
	StarHubPlan,
	StarStation
};

class IMessage
{
public:
	virtual ~IMessage() = default;

	virtual MessageType type() const = 0;
};

} // namespace starTopologyEmulator
