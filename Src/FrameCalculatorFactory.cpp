#include "StarTopologyEmulator/FrameCalculatorFactory.h"

#include "FrameCalculators/AbonentFrameCalculator.h"
#include "FrameCalculators/HubFrameCalculator.h"

namespace starTopologyEmulator
{

std::unique_ptr<IFrameCalculator> FrameCalculatorFactory::abonentCalculator(FrameConfig config, Timestamp tts)
{
	return std::make_unique<AbonentFrameCalculator>(std::move(config), tts);
}

std::unique_ptr<IFrameCalculator> FrameCalculatorFactory::hubCalculator(FrameConfig config, Timestamp tts)
{
	return std::make_unique<HubFrameCalculator>(std::move(config), tts);
}

} // namespace starTopologyEmulator
