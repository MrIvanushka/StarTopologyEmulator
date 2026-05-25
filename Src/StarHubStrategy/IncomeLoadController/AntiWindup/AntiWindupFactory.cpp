#include "AntiWindupFactory.h"

#include "BackCalculationAntiWindup.h"
#include "ClampingAntiWindup.h"
#include "NoneAntiWindup.h"
#include "TrackingModeAntiWindup.h"

namespace starTopologyEmulator
{

std::unique_ptr<IAntiWindup> AntiWindupFactory::make(const PiAntiWindupConfig& config)
{
	return std::visit(
		[](const auto& cfg) -> std::unique_ptr<IAntiWindup>
		{
			using T = std::decay_t<decltype(cfg)>;
			if constexpr (std::is_same_v<T, NoneAntiWindupConfig>)
				return std::make_unique<NoneAntiWindup>();
			else if constexpr (std::is_same_v<T, ClampingAntiWindupConfig>)
				return std::make_unique<ClampingAntiWindup>();
			else if constexpr (std::is_same_v<T, BackCalculationAntiWindupConfig>)
				return std::make_unique<BackCalculationAntiWindup>(cfg);
			else if constexpr (std::is_same_v<T, TrackingModeAntiWindupConfig>)
				return std::make_unique<TrackingModeAntiWindup>();
		},
		config);
}

} // namespace starTopologyEmulator
