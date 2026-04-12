#pragma once

#include <utility>

#include "StarTopologyEmulator/IFaces/IDynamicFrameSettings.h"
#include "StarTopologyEmulator/IFaces/IIncomeLoadController.h"
#include "StarTopologyEmulator/IFaces/IIncomeStationsPredictor.h"
#include "StarTopologyEmulator/StarHubStrategy/IncomeLoadController/HysteresisLoadControllerConfig.h"

namespace starTopologyEmulator
{

class HysteresisLoadController : public IIncomeLoadController
{
	enum class State
	{
		Normal,
		Critical
	};

public:
	explicit HysteresisLoadController(
		std::shared_ptr<IDynamicFrameSettings>,
		std::shared_ptr<IIncomeStationsPredictor>,
		HysteresisLoadControllerConfig&&);

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

	State nextStateByHysteresis(State current, double predictedHoldLoad) const;

	std::pair<double, std::uint32_t> targetCommandForState(State state) const;

	double smoothProbability(double current, double target) const;

	std::uint32_t smoothBackoff(std::uint32_t current, std::uint32_t target) const;

private:
	const HysteresisLoadControllerConfig _config;

	std::shared_ptr<IDynamicFrameSettings> _dynamicFrameSettings;
	std::shared_ptr<IIncomeStationsPredictor> _readyUsersPredictor;

	State _state = State::Normal;
};

} // namespace starTopologyEmulator
