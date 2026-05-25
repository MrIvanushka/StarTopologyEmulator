#include "NoneAntiWindup.h"

namespace starTopologyEmulator
{

double NoneAntiWindup::correction(const Step&)
{
	return 0.0;
}

} // namespace starTopologyEmulator
