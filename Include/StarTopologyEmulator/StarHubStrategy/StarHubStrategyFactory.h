#pragma once

#include <memory>

#include "StarTopologyEmulator/IFaces/IStarHubStrategy.h"
#include "StarTopologyEmulator/IFaces/IFtpGenerator.h"
#include "StarTopologyEmulator/IFaces/IIncomeLoadController.h"
#include "StarTopologyEmulator/IFaces/IIncomeLoadEstimator.h"
#include "StarTopologyEmulator/StarHubStrategy/StarHubStrategyConfig.h"

namespace starTopologyEmulator
{

class STAR_TOPOLOGY_EMULATOR_LIB_EXPORT StarHubStrategyFactory
{
public:
	static std::unique_ptr<IStarHubStrategy> make(
		std::unique_ptr<IFtpGenerator>,
		std::unique_ptr<IIncomeLoadController>);

    static std::unique_ptr<IStarHubStrategy> make(
		std::shared_ptr<IIncomeLoadEstimator>,
		StarHubStrategyConfig&&);
};

} // namespace starTopologyEmulator
