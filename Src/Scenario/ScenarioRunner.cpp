#include "ScenarioRunner.h"

#include <algorithm>

#include "StarTopologyEmulator/IFaces/IEmulator.h"
#include "StarTopologyEmulator/IFaces/IScenarioEvent.h"

namespace starTopologyEmulator
{

ScenarioRunner::ScenarioRunner(std::shared_ptr<IEmulator> emulator)
	: _emulator(std::move(emulator))
{}

std::shared_ptr<IEmulator> ScenarioRunner::emulator() const
{
	return _emulator;
}

void ScenarioRunner::load(std::vector<std::unique_ptr<IScenarioEvent>> events)
{
	_events = std::move(events);
	std::sort(_events.begin(), _events.end(),
		[](const auto& a, const auto& b) { return a->at() < b->at(); });
	_cursor = 0;
}

std::vector<ScenarioError> ScenarioRunner::prepare() const
{
	std::vector<ScenarioError> errors;
	for (const auto& event : _events)
	{
		if (event->at() < 0)
			errors.push_back({ event->at(), "event timestamp is negative" });
	}
	return errors;
}

void ScenarioRunner::update(Timestamp currentTime)
{
	while (_cursor < _events.size() && _events[_cursor]->at() <= currentTime)
	{
		const auto& event = *_events[_cursor];
		if (_callback)
			_callback(currentTime, event);
		event.apply(*_emulator);
		++_cursor;
	}

	_emulator->update(static_cast<IEmulator::Timestamp>(currentTime));
}

bool ScenarioRunner::finished() const
{
	return _cursor >= _events.size();
}

std::size_t ScenarioRunner::totalEvents() const
{
	return _events.size();
}

std::size_t ScenarioRunner::consumedEvents() const
{
	return _cursor;
}

std::optional<Timestamp> ScenarioRunner::nextEventAt() const
{
	if (_cursor < _events.size())
		return _events[_cursor]->at();
	return std::nullopt;
}

void ScenarioRunner::reset()
{
	_cursor = 0;
}

void ScenarioRunner::setEventAppliedCallback(EventAppliedCallback callback)
{
	_callback = std::move(callback);
}

} // namespace starTopologyEmulator
