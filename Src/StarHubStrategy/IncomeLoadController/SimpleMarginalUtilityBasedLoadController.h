#pragma once

#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

#include "StarTopologyEmulator/IFaces/IDynamicFrameSettings.h"
#include "StarTopologyEmulator/IFaces/IIncomeLoadController.h"
#include "StarTopologyEmulator/IFaces/IIncomeStationsPredictor.h"
#include "StarTopologyEmulator/StarHubStrategy/IncomeLoadController/SimpleMarginalUtilityBasedLoadControllerConfig.h"

namespace starTopologyEmulator
{

class SimpleMarginalUtilityBasedLoadController : public IIncomeLoadController
{
public:
	explicit SimpleMarginalUtilityBasedLoadController(
		std::shared_ptr<IDynamicFrameSettings>,
		std::shared_ptr<IIncomeStationsPredictor>,
		SimpleMarginalUtilityBasedLoadControllerConfig&&);

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

	double predictLoadUnderCommand(
		double predictedReadyUsers,
		std::uint64_t raSlots,
		double txProbability,
		std::uint32_t backoffWindowFrames) const;

	double successPerSlotSa(double g) const;

	double collisionProbabilitySa(double g) const;

	double predictedDelayFrames(
		double predictedReadyUsers,
		std::uint64_t raSlots,
		double txProbability,
		std::uint32_t backoffWindowFrames) const;

	double utility(
		double predictedReadyUsers,
		std::uint64_t raSlots,
		double txProbability,
		std::uint32_t backoffWindowFrames) const;

	double smoothProbability(double current, double target) const;

	std::uint32_t smoothBackoff(std::uint32_t current, std::uint32_t target) const;

	std::vector<std::uint32_t> candidateBackoffWindows(std::uint32_t current) const;

private:
	const SimpleMarginalUtilityBasedLoadControllerConfig _config;

	std::shared_ptr<IDynamicFrameSettings> _dynamicFrameSettings;
	std::shared_ptr<IIncomeStationsPredictor> _readyUsersPredictor;
};

} // namespace starTopologyEmulator