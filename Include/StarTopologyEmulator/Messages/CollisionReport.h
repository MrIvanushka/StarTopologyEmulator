#pragma once

#include "StarTopologyEmulator/IFaces/IMessage.h"
#include "StarTopologyEmulator/StarTopologyEmulatorExport.h"

namespace starTopologyEmulator
{

class STAR_TOPOLOGY_EMULATOR_LIB_EXPORT CollisionReport : public IMessage
{
public:
	MessageType type() const override { return MessageType::CollisionReport; }
};

} // namespace starTopologyEmulator
