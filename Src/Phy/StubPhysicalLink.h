#pragma once

#include "StarTopologyEmulator/IFaces/IPhysicalLink.h"
#include "StarTopologyEmulator/Phy/StubPhysicalLinkConfig.h"

namespace starTopologyEmulator
{

class StubPhysicalLink : public IPhysicalLink
{
public:
	explicit StubPhysicalLink(StubPhysicalLinkConfig config);

	std::size_t infoBitCount() const override;

	std::size_t symbolCount() const override;

	SymbolVector modulate(const BitVector& payload, Symbol gain) const override;

	DecodeResult demodulateDecode(const SymbolVector& received, float noiseVariance) const override;

private:
	StubPhysicalLinkConfig _config;
};

} // namespace starTopologyEmulator
