#pragma once

#include <string>
#include <vector>

#include "StarTopologyEmulator/CommonTypedefs.h"

namespace starTopologyEmulator
{

struct MetricValue
{
	MetricValue(std::string n, double v)
		: name(std::move(n)), value(v)
	{ }

	const std::string name;
	const double value;
};

struct MetricValueFolder
{
	MetricValueFolder() = default;

	MetricValueFolder(
		std::string n,
		std::vector<MetricValueFolder> sf,
		std::vector<MetricValue> ms) 
		: name(std::move(n)), subfolders(std::move(sf)), metrics(std::move(ms))
	{}

	const std::string name;
	const std::vector<MetricValueFolder> subfolders;
	const std::vector<MetricValue> metrics;
};

} // namespace starTopologyEmulator
