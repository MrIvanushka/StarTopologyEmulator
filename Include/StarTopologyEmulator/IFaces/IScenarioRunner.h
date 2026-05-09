#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "StarTopologyEmulator/CommonTypedefs.h"
#include "StarTopologyEmulator/StarTopologyEmulatorExport.h"

namespace starTopologyEmulator
{

class IEmulator;
class IScenarioEvent;

struct STAR_TOPOLOGY_EMULATOR_LIB_EXPORT ScenarioError
{
	Timestamp at = -1;
	std::string message;
};

class IScenarioRunner
{
public:
	using EventAppliedCallback =
		std::function<void(Timestamp now, const IScenarioEvent& event)>;

	virtual ~IScenarioRunner() = default;

	virtual std::shared_ptr<IEmulator> emulator() const = 0;

	virtual void load(std::vector<std::unique_ptr<IScenarioEvent>> events) = 0;

	virtual std::vector<ScenarioError> prepare() const = 0;

	virtual void update(Timestamp currentTime) = 0;

	virtual bool finished() const = 0;

	virtual std::size_t totalEvents() const = 0;
	virtual std::size_t consumedEvents() const = 0;
	virtual std::optional<Timestamp> nextEventAt() const = 0;

	virtual void reset() = 0;

	virtual void setEventAppliedCallback(EventAppliedCallback) = 0;
};

} // namespace starTopologyEmulator
