#pragma once

#include <utility>

#include "Metrics/Metrics.h"
#include "StarTopologyEmulator/IFaces/IStarHubStrategy.h"
#include "StarTopologyEmulator/IFaces/IFtpGenerator.h"
#include "StarTopologyEmulator/IFaces/IIncomeLoadController.h"
#include "StarTopologyEmulator/StarHubStrategy/StarHubStrategyConfig.h"

namespace starTopologyEmulator
{

class CommonStarHubStrategy : public IStarHubStrategy
{
	DECLARE_METRICS("Стратегия случайного доступа")
public:
	using Config = StarHubStrategyConfig;

	CommonStarHubStrategy(
		std::unique_ptr<IFtpGenerator>,
		std::unique_ptr<IIncomeLoadController>);

	std::shared_ptr<StarHubPlanMessage> generate(std::uint64_t currentFrame, std::uint64_t targetFrame) override;
private:
	std::unique_ptr<IFtpGenerator> _ftpGenerator;
	std::unique_ptr<IIncomeLoadController> _incomeLoadController;

	std::shared_ptr<StarHubPlanMessage> _lastPlan;
};

} // namespace starTopologyEmulator
