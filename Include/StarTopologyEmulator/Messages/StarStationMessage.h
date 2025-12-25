#pragma once

#include <cstdint>

#include "StarTopologyEmulator/CommonTypedefs.h"
#include "StarTopologyEmulator/IFaces/IMessage.h"
#include "StarTopologyEmulator/StarTopologyEmulatorExport.h"

namespace starTopologyEmulator
{

class STAR_TOPOLOGY_EMULATOR_LIB_EXPORT StarStationMessage : public IMessage
{
public:
	StarStationMessage(int id) : _stationID(id)
	{}

	MessageType type() const override { return MessageType::StarStation; }

	StationID stationID() const { return _stationID; }
private:
	StationID _stationID = 0;
};

} // namespace starTopologyEmulator
