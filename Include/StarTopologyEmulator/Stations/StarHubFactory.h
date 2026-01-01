#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "StarTopologyEmulator/CommonTypedefs.h"
#include "StarTopologyEmulator/IFaces/IFrameCalculator.h"
#include "StarTopologyEmulator/IFaces/IIncomeLoadEstimator.h"
#include "StarTopologyEmulator/IFaces/IStarHub.h"
#include "StarTopologyEmulator/IFaces/IStarHubStrategy.h"

namespace starTopologyEmulator
{

class STAR_TOPOLOGY_EMULATOR_LIB_EXPORT StarHubFactory
{
public:
	static std::shared_ptr<IStarHub> make(
		std::function<void(Timestamp, std::shared_ptr<IMessage>)> sendFunc,
		std::unique_ptr<IIncomeLoadEstimator> incomeLoadEstimator,
		std::unique_ptr<IFrameCalculator> frameCalculator,
		std::unique_ptr<IStarHubStrategy> strategy,
		Timestamp tts);
};

} // namespace starTopologyEmulator
