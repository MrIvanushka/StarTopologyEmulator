#include "TrackingModeAntiWindup.h"

namespace starTopologyEmulator
{

void TrackingModeAntiWindup::reset()
{
	_wasSaturated = false;
}

double TrackingModeAntiWindup::correction(const Step& step)
{
	const bool isSaturated = step.pUnclamped != step.pClamped;

	double corr = 0.0;
	if (_wasSaturated && !isSaturated)
	{
		const double targetIntegral = step.pClamped - step.kP * step.error;
		corr = step.integral + step.kI * step.error - targetIntegral;
	}

	_wasSaturated = isSaturated;
	return corr;
}

} // namespace starTopologyEmulator
