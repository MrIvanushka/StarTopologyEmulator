#include "ParetoBurstTrafficProfile.h"

#include <algorithm>
#include <cmath>

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

ParetoBurstTrafficProfile::ParetoBurstTrafficProfile(ParetoBurstTrafficProfileConfig cfg)
	: _cfg(cfg)
	, _rng(makeRng(cfg.seed))
{
	_valid = _cfg.packetsPerTimestampOn > 0.0
		&& _cfg.bitsPerPacket > 0
		&& _cfg.minOnDuration > 0.0
		&& _cfg.minOffDuration > 0.0
		&& _cfg.alpha > 1.0;

	if (!_valid)
		return;

	const double pOn = _cfg.minOnDuration / (_cfg.minOnDuration + _cfg.minOffDuration);
	std::uniform_real_distribution<double> coin(0.0, 1.0);
	_phase = (coin(_rng) < pOn) ? Phase::On : Phase::Off;
	_remainingInPhase = sampleResidual(_phase);
}

double ParetoBurstTrafficProfile::sampleParetoDuration(double xm)
{
	std::uniform_real_distribution<double> u(0.0, 1.0);
	return xm / std::pow(1.0 - u(_rng), 1.0 / _cfg.alpha);
}

double ParetoBurstTrafficProfile::sampleResidual(Phase phase)
{
	const double xm = (phase == Phase::On) ? _cfg.minOnDuration : _cfg.minOffDuration;
	return sampleParetoDuration(xm);
}

std::uint64_t ParetoBurstTrafficProfile::generateBits(Timestamp duration)
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
