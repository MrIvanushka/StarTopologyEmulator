#include "StaticIncomeLoadController.h"

namespace starTopologyEmulator
{

StaticIncomeLoadController::StaticIncomeLoadController(
	StarHubPlanMessage::BackoffConfig config)
	: _config(config)
{}

StarHubPlanMessage::BackoffConfig StaticIncomeLoadController::generate(
	std::uint64_t plannedRaSlots,
	std::uint64_t currentFrame,
	std::uint64_t targetFrame)
{
	return _config;
}

} // namespace starTopologyEmulator
