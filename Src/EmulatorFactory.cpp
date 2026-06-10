#include "StarTopologyEmulator/EmulatorFactory.h"

#include "Emulator.h"
#include "CollisionResolver/SimpleSaCollisionResolver.h"

namespace starTopologyEmulator
{

std::unique_ptr<IEmulator> EmulatorFactory::make(EmulatorInitData initData)
{
	auto collisionResolver = std::move(initData.collisionResolver);
	if (!collisionResolver)
		collisionResolver = std::make_unique<SimpleSaCollisionResolver>();

	return std::make_unique<Emulator>(
		initData.stationFactory,
		initData.hubFactory,
		std::move(initData.abonentFrameCalculator),
		std::move(initData.hubFrameCalculator),
		std::move(collisionResolver),
		initData.stationCount,
		std::move(initData.metricSink));
}

} // namespace starTopologyEmulator
