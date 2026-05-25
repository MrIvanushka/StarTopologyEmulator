#include "BackCalculationAntiWindup.h"

namespace starTopologyEmulator
{

BackCalculationAntiWindup::BackCalculationAntiWindup(BackCalculationAntiWindupConfig config)
	: _config(config)
{
}

double BackCalculationAntiWindup::correction(const Step& step)
{
	return _config.kT * (step.pUnclamped - step.pClamped);
}

} // namespace starTopologyEmulator
