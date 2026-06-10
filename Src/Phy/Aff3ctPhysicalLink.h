#pragma once

#ifdef STE_WITH_AFF3CT

#include <memory>

#include "StarTopologyEmulator/IFaces/IPhysicalLink.h"
#include "StarTopologyEmulator/Phy/Aff3ctPhysicalLinkConfig.h"

namespace starTopologyEmulator
{

class Aff3ctPhysicalLink : public IPhysicalLink
{
public:
	explicit Aff3ctPhysicalLink(Aff3ctPhysicalLinkConfig config);
	~Aff3ctPhysicalLink() override;

	std::size_t infoBitCount() const override;

	std::size_t symbolCount() const override;

	SymbolVector modulate(const BitVector& payload, Symbol gain) const override;

	DecodeResult demodulateDecode(const SymbolVector& received, float noiseVariance) const override;

private:
	struct Impl;
	std::unique_ptr<Impl> _impl;
};

} // namespace starTopologyEmulator

#endif // STE_WITH_AFF3CT
