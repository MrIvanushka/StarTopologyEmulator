#pragma once

#include <optional>
#include <utility>

#include "Metrics/Metrics.h"
#include "StarTopologyEmulator/IFaces/IIncomeStationsPredictor.h"
#include "StarTopologyEmulator/IFaces/IDynamicFrameSettings.h"
#include "StarTopologyEmulator/IFaces/IFrameCalculator.h"
#include "StarTopologyEmulator/IFaces/IIncomeLoadEstimator.h"

namespace starTopologyEmulator
{

class CogorthyIncomeStationsPredictor : public IIncomeStationsPredictor
{
public:
	explicit CogorthyIncomeStationsPredictor(
		std::shared_ptr<IIncomeLoadEstimator> incomeLoadEstimator,
		std::shared_ptr<IDynamicFrameSettings> dynamicFrameSettings);

	double estimateReadyUsers(
		std::uint64_t currentFrame,
		std::uint64_t targetFrame) override;
private:
	double calculateImpact(
		std::uint64_t impactFrame,
		std::uint64_t targetFrame,
		std::uint64_t commandApplyDelay);

	std::unordered_map<std::uint64_t, double> _incomeLoadHistory;
	std::shared_ptr<IIncomeLoadEstimator> _incomeLoadEstimator;
	std::shared_ptr<IDynamicFrameSettings> _dynamicFrameSettings;
};

} // namespace starTopologyEmulator
