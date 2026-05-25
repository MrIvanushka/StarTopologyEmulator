#pragma once

namespace starTopologyEmulator
{

class IAntiWindup
{
public:
	virtual ~IAntiWindup() = default;

	struct Step
	{
		double integral;
		double pUnclamped;
		double pClamped;
		double error;
		double kP;
		double kI;
	};

	virtual void reset() {}

	virtual double correction(const Step& step) = 0;
};

} // namespace starTopologyEmulator
