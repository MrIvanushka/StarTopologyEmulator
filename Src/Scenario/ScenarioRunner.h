#pragma once

#include <cstddef>
#include <memory>
#include <vector>

#include "StarTopologyEmulator/IFaces/IScenarioRunner.h"

namespace starTopologyEmulator
{

class ScenarioRunner : public IScenarioRunner
{
public:
	explicit ScenarioRunner(std::shared_ptr<IEmulator> emulator);

	std::shared_ptr<IEmulator> emulator() const override;

	void load(std::vector<std::unique_ptr<IScenarioEvent>> events) override;
	std::vector<ScenarioError> prepare() const override;

	void update(Timestamp currentTime) override;
	bool finished() const override;

	std::size_t totalEvents() const override;
	std::size_t consumedEvents() const override;
	std::optional<Timestamp> nextEventAt() const override;

	void reset() override;

	void setEventAppliedCallback(EventAppliedCallback callback) override;

private:
	std::shared_ptr<IEmulator> _emulator;
	std::vector<std::unique_ptr<IScenarioEvent>> _events;
	std::size_t _cursor{ 0 };
	EventAppliedCallback _callback;
};

} // namespace starTopologyEmulator
