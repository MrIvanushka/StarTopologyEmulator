#include "CbrTrafficProfile.h"

#include <algorithm>
#include <cmath>

namespace starTopologyEmulator
{

CbrTrafficProfile::CbrTrafficProfile(CbrTrafficProfileConfig cfg)
	: _cfg(cfg)
{}

std::uint64_t CbrTrafficProfile::generateBits(Timestamp duration)
{
	if (duration <= 0 || _cfg.bitsPerTimestamp <= 0.0)
		return 0;

	const double bits = _cfg.bitsPerTimestamp * static_cast<double>(duration) + _residual;
	const double whole = std::floor(bits);
	_residual = bits - whole;

	return static_cast<std::uint64_t>(whole);
}

} // namespace starTopologyEmulator
