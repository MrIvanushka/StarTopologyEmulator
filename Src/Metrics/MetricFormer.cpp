#include "MetricFormer.h"

using namespace starTopologyEmulator;

MetricFormer::MetricFormer(
	std::string name, 
	std::function<double()> getter)
	: _name(name), _getter(getter)
{}

MetricValue MetricFormer::makeValue() const
{
	return MetricValue(_name, _getter());
}
