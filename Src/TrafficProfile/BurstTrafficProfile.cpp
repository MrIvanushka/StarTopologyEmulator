#include "BurstTrafficProfile.h"

#include <algorithm>

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

BurstTrafficProfile::BurstTrafficProfile(BurstTrafficProfileConfig cfg)
	: _cfg(cfg)
	, _rng(makeRng(cfg.seed))
{
	_valid = _cfg.packetsPerTimestampOn > 0.0
		&& _cfg.bitsPerPacket > 0
		&& _cfg.meanOnDuration > 0.0
		&& _cfg.meanOffDuration > 0.0;

	if (!_valid)
		return;

	const double pOn = _cfg.meanOnDuration / (_cfg.meanOnDuration + _cfg.meanOffDuration);
	std::uniform_real_distribution<double> coin(0.0, 1.0);
	_phase = (coin(_rng) < pOn) ? Phase::On : Phase::Off;
	_remainingInPhase = sampleResidual(_phase);
}

double BurstTrafficProfile::sampleResidual(Phase phase)
{
	const double mean = (phase == Phase::On) ? _cfg.meanOnDuration : _cfg.meanOffDuration;
	std::exponential_distribution<double> dist(1.0 / mean);
	return dist(_rng);
}

std::uint64_t BurstTrafficProfile::generateBits(Timestamp duration)
{
	if (!_valid || duration <= 0)
		return 0;

	std::uint64_t packets = 0;
	double remaining = static_cast<double>(duration);

	while (remaining > 0.0)
	{
		if (_remainingInPhase <= 0.0)
		{
			_phase = (_phase == Phase::On) ? Phase::Off : Phase::On;
			_remainingInPhase = sampleResidual(_phase);
		}

		const double dt = std::min(remaining, _remainingInPhase);
		if (_phase == Phase::On)
		{
			std::poisson_distribution<std::uint64_t> pdist(_cfg.packetsPerTimestampOn * dt);
			packets += pdist(_rng);
		}

		_remainingInPhase -= dt;
		remaining -= dt;
	}

	return packets * _cfg.bitsPerPacket;
}

} // namespace starTopologyEmulator
