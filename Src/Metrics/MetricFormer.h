#pragma once

#include <functional>
#include <string>

#include "StarTopologyEmulator/Metrics/MetricValue.h"

namespace starTopologyEmulator
{

class MetricFormer
{
public:
	MetricFormer(std::string name, std::function<double()> getter);

	MetricValue makeValue() const;
private:
	std::string _name;

	std::function<double()> _getter;
};

} // namespace starTopologyEmulator
