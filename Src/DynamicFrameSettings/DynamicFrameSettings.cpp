#include "StarTopologyEmulator/DynamicFrameSettingsFactory.h"

#include "DynamicFrameSettings/DynamicFrameSettings.h"

namespace starTopologyEmulator
{

std::unique_ptr<IDynamicFrameSettings> DynamicFrameSettingsFactory::make(int maxPlansStored)
{
	return std::make_unique<DynamicFrameSettings>(maxPlansStored);
}

} // namespace starTopologyEmulator
