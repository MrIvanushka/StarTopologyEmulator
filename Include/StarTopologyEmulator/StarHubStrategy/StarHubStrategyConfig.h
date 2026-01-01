#pragma once

#include <cstdint>

namespace starTopologyEmulator
{

struct StarHubStrategyConfig
{
	std::uint8_t totalSlots = 100;
	std::uint8_t minRaSlots = 5;
	std::uint8_t maxRaSlots = 50;

	double targetPlr = 0.1;
	double heavyLoadG = 0.7;

	std::uint8_t minBaseWindow = 1;
	std::uint8_t maxBaseWindow = 8;
	double minPTx = 0.2;
	double maxPTx = 1.0;
};

} // namespace starTopologyEmulator
