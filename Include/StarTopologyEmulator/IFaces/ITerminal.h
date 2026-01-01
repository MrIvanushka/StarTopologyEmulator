#pragma once

#include <cstdint>
#include <memory>

#include "StarTopologyEmulator/IFaces/IMessage.h"
#include "StarTopologyEmulator/IFaces/IMetricProducer.h"

namespace starTopologyEmulator
{

class ITerminal : public IMetricProducer
{
public:
	virtual void update(Timestamp) = 0;

	virtual void handleMessage(std::shared_ptr<IMessage>, Timestamp) = 0;

	virtual Timestamp tts() const = 0;
};

} // namespace starTopologyEmulator
