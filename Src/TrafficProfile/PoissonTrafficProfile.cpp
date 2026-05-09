#include "PoissonTrafficProfile.h"

namespace starTopologyEmulator
{

namespace
{

std::mt19937 makeRng(std::uint32_t seed)
{
	if (seed != 0)
		return std::mt19937(seed);

	std::random_device rd;
	return std::mt19937(rd());
}

} // namespace

PoissonTrafficProfile::PoissonTrafficProfile(PoissonTrafficProfileConfig cfg)
	: _cfg(cfg)
	, _rng(makeRng(cfg.seed))
{}

std::uint64_t PoissonTrafficProfile::generateBits(Timestamp duration)
{
	if (duration <= 0 || _cfg.packetsPerTimestamp <= 0.0 || _cfg.bitsPerPacket == 0)
		return 0;

	const double mean = _cfg.packetsPerTimestamp * static_cast<double>(duration);
	std::poisson_distribution<std::uint64_t> dist(mean);
	const std::uint64_t packets = dist(_rng);

	return packets * _cfg.bitsPerPacket;
}

} // namespace starTopologyEmulator
