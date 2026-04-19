#pragma once

#include <memory>
#include <utility>

#include "Metrics/Metrics.h"
#include "StarTopologyEmulator/IFaces/IDynamicFrameSettings.h"
#include "StarTopologyEmulator/IFaces/IIncomeLoadController.h"
#include "StarTopologyEmulator/IFaces/IIncomeStationsPredictor.h"
#include "StarTopologyEmulator/StarHubStrategy/IncomeLoadController/TargetLoadControllerConfig.h"

namespace starTopologyEmulator
{

class TargetLoadController : public IIncomeLoadController
{
	DECLARE_METRICS("Контроллер входной нагрузки");
public:
	explicit TargetLoadController(
		std::shared_ptr<IDynamicFrameSettings>,
		std::shared_ptr<IIncomeStationsPredictor>,
		TargetLoadControllerConfig&&);

	StarHubPlanMessage::BackoffConfig generate(
		std::uint64_t plannedRaSlots,
		std::uint64_t currentFrame,
		std::uint64_t targetFrame) override;

private:
	double clampProbability(double value) const;

	std::uint32_t clampBackoff(std::uint32_t value) const;

	double backoffEligibilityFactor(std::uint32_t windowFrames) const;

	double effectiveAggressiveness(
		double txProbability,
		std::uint32_t backoffWindowFrames) const;

	double minAggressiveness() const;

	double maxAggressiveness() const;

	double predictLoadUnderCommand(
		double predictedReadyUsers,
		std::uint64_t raSlots,
		double txProbability,
		std::uint32_t backoffWindowFrames) const;

	double computeTargetAggressiveness(
		double predictedReadyUsers,
		std::uint64_t plannedRaSlots) const;

	std::pair<double, std::uint32_t> commandForAggressiveness(
		double targetAggressiveness) const;

	double smoothProbability(double current, double target) const;

	std::uint32_t smoothBackoff(std::uint32_t current, std::uint32_t target) const;

private:
	const TargetLoadControllerConfig _config;

	std::shared_ptr<IDynamicFrameSettings> _dynamicFrameSettings;
	std::shared_ptr<IIncomeStationsPredictor> _readyUsersPredictor;

	std::shared_ptr<StarHubPlanMessage> _currentPlan;
	std::shared_ptr<StarHubPlanMessage> _targetPlan;
};

} // namespace starTopologyEmulator
