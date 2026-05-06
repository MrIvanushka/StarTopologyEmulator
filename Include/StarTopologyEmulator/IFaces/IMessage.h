#pragma once

namespace starTopologyEmulator
{

enum class MessageType
{
	StarHubAccess,
	StarHubPlan,
	StarStation,
	CollisionReport,
	OperationPlan,
	BacklogReport
};

class IMessage
{
public:
	virtual ~IMessage() = default;

	virtual MessageType type() const = 0;

	// Messages marked collision-immune are delivered to the hub regardless of
	// concurrent transmissions in the same uplink slot.
	virtual bool isCollisionImmune() const { return false; }
};

} // namespace starTopologyEmulator
