#pragma once

#include <complex>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace starTopologyEmulator
{

class IPhysicalLink
{
public:
	using Bit = std::uint8_t;
	using Symbol = std::complex<float>;
	using BitVector = std::vector<Bit>;
	using SymbolVector = std::vector<Symbol>;

	struct DecodeResult
	{
		BitVector payload;
		Symbol estimatedGain{ 0.0f, 0.0f };
		bool crcOk = false;
	};

	virtual ~IPhysicalLink() = default;

	virtual std::size_t infoBitCount() const = 0;

	virtual std::size_t symbolCount() const = 0;

	virtual SymbolVector modulate(const BitVector& payload, Symbol gain) const = 0;

	virtual DecodeResult demodulateDecode(const SymbolVector& received, float noiseVariance) const = 0;
};

} // namespace starTopologyEmulator
