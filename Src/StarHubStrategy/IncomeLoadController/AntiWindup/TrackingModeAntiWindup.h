#pragma once

#include "StarTopologyEmulator/IFaces/IAntiWindup.h"

namespace starTopologyEmulator
{

class TrackingModeAntiWindup : public IAntiWindup
{
public:
	void reset() override;
	double correction(const Step&) override;

private:
	bool _wasSaturated = false;
};

} // namespace starTopologyEmulator
