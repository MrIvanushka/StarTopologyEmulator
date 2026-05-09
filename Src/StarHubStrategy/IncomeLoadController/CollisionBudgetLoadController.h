#pragma once

#include <memory>

#include "StarTopologyEmulator/IFaces/IDynamicFrameSettings.h"
#include "StarTopologyEmulator/IFaces/IIncomeLoadController.h"
#include "StarTopologyEmulator/IFaces/IIncomeStationsPredictor.h"
#include "StarTopologyEmulator/Metrics/MetricSink.h"
#include "StarTopologyEmulator/StarHubStrategy/IncomeLoadController/CollisionBudgetLoadControllerConfig.h"

namespace starTopologyEmulator
{

class CollisionBudgetLoadController : public IIncomeLoadController
{
public:
	CollisionBudgetLoadController(
		std::shared_ptr<IDynamicFrameSettings>,
		std::shared_ptr<IIncomeStationsPredictor>,
		CollisionBudgetLoadControllerConfig&&,
		MetricScope scope = {});

	StarHubPlanMessage::BackoffConfig generate(
		std::uint64_t plannedRaSlots,
		std::uint64_t currentFrame,
		std::uint64_t targetFrame) override;

private:
	double clampProbability(double value) const;
	double clampStep(double delta) const;

	double collisionRate(double p, double n) const;
	double throughputDerivative(double p, double n) const;
	double collisionRateDerivative(double p, double n) const;

	const CollisionBudgetLoadControllerConfig _config;

	std::shared_ptr<IDynamicFrameSettings> _dynamicFrameSettings;
	std::shared_ptr<IIncomeStationsPredictor> _readyUsersPredictor;

	double _lagrangeMultiplier = 0.0;

	MetricScope _scope;
	MetricHandle _hPTx = kInvalidMetricHandle;
	MetricHandle _hMu = kInvalidMetricHandle;
	MetricHandle _hPColl = kInvalidMetricHandle;
};

} // namespace starTopologyEmulator
