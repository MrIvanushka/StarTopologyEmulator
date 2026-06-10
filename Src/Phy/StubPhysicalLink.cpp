#include "StubPhysicalLink.h"

#include <complex>
#include <limits>

namespace starTopologyEmulator
{

StubPhysicalLink::StubPhysicalLink(StubPhysicalLinkConfig config)
	: _config(config)
{ }

std::size_t StubPhysicalLink::infoBitCount() const
{
	return _config.infoBits;
}

std::size_t StubPhysicalLink::symbolCount() const
{
	return _config.infoBits;
}

IPhysicalLink::SymbolVector StubPhysicalLink::modulate(const BitVector& payload, Symbol gain) const
{
	SymbolVector symbols;
	symbols.reserve(payload.size());
	for (const Bit bit : payload)
		symbols.push_back(gain * (bit ? 1.0f : -1.0f));
	return symbols;
}

IPhysicalLink::DecodeResult StubPhysicalLink::demodulateDecode(
	const SymbolVector& received,
	float noiseVariance) const
{
	(void)noiseVariance;

	DecodeResult result;
	const std::size_t n = received.size();
	if (n == 0)
		return result;

	result.payload.resize(n);
	std::vector<float> decided(n);
	std::complex<double> correlation{ 0.0, 0.0 };
	for (std::size_t i = 0; i < n; ++i)
	{
		const float symbol = received[i].real() >= 0.0f ? 1.0f : -1.0f;
		decided[i] = symbol;
		result.payload[i] = symbol > 0.0f ? Bit{ 1 } : Bit{ 0 };
		correlation += std::complex<double>(received[i]) * static_cast<double>(symbol);
	}

	const std::complex<double> gain = correlation / static_cast<double>(n);

	double residualEnergy = 0.0;
	for (std::size_t i = 0; i < n; ++i)
	{
		const std::complex<double> ideal = gain * static_cast<double>(decided[i]);
		residualEnergy += std::norm(std::complex<double>(received[i]) - ideal);
	}

	const double signalPower = std::norm(gain);
	const double residualPower = residualEnergy / static_cast<double>(n);
	const double sinr = residualPower > 0.0
		? signalPower / residualPower
		: std::numeric_limits<double>::infinity();

	result.estimatedGain = Symbol{ static_cast<float>(gain.real()), static_cast<float>(gain.imag()) };
	result.crcOk = sinr >= static_cast<double>(_config.decodingSinrThreshold);

	return result;
}

} // namespace starTopologyEmulator
