#pragma once

#include <cstdint>
#include <memory>

#include "StarTopologyEmulator/CommonTypedefs.h"
#include "StarTopologyEmulator/IFaces/IMessage.h"

namespace starTopologyEmulator
{

class ITerminal
{
public:
	virtual ~ITerminal() = default;

	virtual void update(Timestamp) = 0;

	virtual void handleMessage(std::shared_ptr<IMessage>, Timestamp) = 0;

	virtual Timestamp tts() const = 0;
};

} // namespace starTopologyEmulator
