#pragma once

namespace starTopologyEmulator
{

enum class MessageType
{
	StarHubAccess,
	StarHubPlan,
	StarStation,
	CollisionReport
};

class IMessage
{
public:
	virtual ~IMessage() = default;

	virtual MessageType type() const = 0;
};

} // namespace starTopologyEmulator
