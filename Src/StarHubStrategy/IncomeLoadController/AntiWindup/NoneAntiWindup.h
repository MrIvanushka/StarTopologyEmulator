#pragma once

#include "StarTopologyEmulator/IFaces/IAntiWindup.h"

namespace starTopologyEmulator
{

class NoneAntiWindup : public IAntiWindup
{
public:
	double correction(const Step&) override;
};

} // namespace starTopologyEmulator
