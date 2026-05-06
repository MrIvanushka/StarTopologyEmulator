#pragma once

#include <cstdint>

#include "StarTopologyEmulator/CommonTypedefs.h"
#include "StarTopologyEmulator/IFaces/IMessage.h"
#include "StarTopologyEmulator/StarTopologyEmulatorExport.h"

namespace starTopologyEmulator
{

class STAR_TOPOLOGY_EMULATOR_LIB_EXPORT BacklogReportMessage : public IMessage
{
public:
	BacklogReportMessage(StationID id, std::uint64_t backlogBits)
		: _id(id), _backlogBits(backlogBits)
	{}

	MessageType type() const override { return MessageType::BacklogReport; }

	bool isCollisionImmune() const override { return true; }

	StationID stationID() const { return _id; }
	std::uint64_t backlogBits() const { return _backlogBits; }

private:
	StationID _id;
	std::uint64_t _backlogBits;
};

} // namespace starTopologyEmulator
