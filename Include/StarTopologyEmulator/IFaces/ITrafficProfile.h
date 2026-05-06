#pragma once

#include <cstdint>

#include "StarTopologyEmulator/CommonTypedefs.h"

namespace starTopologyEmulator
{

class ITrafficProfile
{
public:
	virtual ~ITrafficProfile() = default;

	// Bits accumulated over the given duration. Implementations may keep a
	// fractional residual across calls so total bits are not lost to rounding.
	virtual std::uint64_t generateBits(Timestamp duration) = 0;
};

} // namespace starTopologyEmulator
