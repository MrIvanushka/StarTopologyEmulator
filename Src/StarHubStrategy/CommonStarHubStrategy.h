#pragma once

#include <utility>

#include "StarTopologyEmulator/IFaces/IStarHubStrategy.h"
#include "StarTopologyEmulator/IFaces/IFtpGenerator.h"
#include "StarTopologyEmulator/IFaces/IIncomeLoadController.h"
#include "StarTopologyEmulator/Metrics/MetricSink.h"
#include "StarTopologyEmulator/StarHubStrategy/StarHubStrategyConfig.h"

namespace starTopologyEmulator
{

class CommonStarHubStrategy : public IStarHubStrategy
{
public:
	using Config = StarHubStrategyConfig;

	CommonStarHubStrategy(
		std::unique_ptr<IFtpGenerator>,
		std::unique_ptr<IIncomeLoadController>,
		MetricScope scope = {});

	std::shared_ptr<StarHubPlanMessage> generate(std::uint64_t currentFrame, std::uint64_t targetFrame) override;
private:
	std::unique_ptr<IFtpGenerator> _ftpGenerator;
	std::unique_ptr<IIncomeLoadController> _incomeLoadController;

	std::shared_ptr<StarHubPlanMessage> _lastPlan = std::make_shared<StarHubPlanMessage>(
		0,
		StarHubPlanMessage::FtpConfig(),
		StarHubPlanMessage::BackoffConfig());

	MetricScope _scope;
	MetricHandle _hBaseWindow = kInvalidMetricHandle;
	MetricHandle _hMaxWindow = kInvalidMetricHandle;
	MetricHandle _hPTx = kInvalidMetricHandle;
	MetricHandle _hRaSlots = kInvalidMetricHandle;
};

} // namespace starTopologyEmulator
