#include "ClampingAntiWindup.h"

namespace starTopologyEmulator
{

double ClampingAntiWindup::correction(const Step& step)
{
	const bool saturatedHigh = step.pUnclamped > step.pClamped;
	const bool saturatedLow = step.pUnclamped < step.pClamped;

	const bool freeze =
		(saturatedHigh && step.error > 0.0) ||
		(saturatedLow && step.error < 0.0);

	return freeze ? step.kI * step.error : 0.0;
}

} // namespace starTopologyEmulator
