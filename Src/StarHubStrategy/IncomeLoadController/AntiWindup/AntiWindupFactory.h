#pragma once

#include <memory>

#include "StarTopologyEmulator/IFaces/IAntiWindup.h"
#include "StarTopologyEmulator/StarHubStrategy/IncomeLoadController/AntiWindup/PiAntiWindupConfig.h"

namespace starTopologyEmulator
{

class AntiWindupFactory
{
public:
	static std::unique_ptr<IAntiWindup> make(const PiAntiWindupConfig&);
};

} // namespace starTopologyEmulator
