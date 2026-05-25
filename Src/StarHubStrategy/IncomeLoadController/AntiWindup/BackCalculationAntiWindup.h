#pragma once

#include "StarTopologyEmulator/IFaces/IAntiWindup.h"
#include "StarTopologyEmulator/StarHubStrategy/IncomeLoadController/AntiWindup/BackCalculationAntiWindupConfig.h"

namespace starTopologyEmulator
{

class BackCalculationAntiWindup : public IAntiWindup
{
public:
	explicit BackCalculationAntiWindup(BackCalculationAntiWindupConfig);

	double correction(const Step&) override;

private:
	const BackCalculationAntiWindupConfig _config;
};

} // namespace starTopologyEmulator
