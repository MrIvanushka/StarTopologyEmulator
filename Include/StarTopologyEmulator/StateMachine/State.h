#pragma once

#include "StarTopologyEmulator/StarTopologyEmulatorExport.h"
#include "StarTopologyEmulator/StateMachine/IFaces/IState.h"
#include "StarTopologyEmulator/StateMachine/IFaces/ITransition.h"

#include <memory>
#include <vector>

namespace starTopologyEmulator
{

class STAR_TOPOLOGY_EMULATOR_LIB_EXPORT State : public IState
{
public:
	using Transitions = std::vector<std::shared_ptr<ITransition>>;

	State(const Transitions& transitions, Index index);

	NextStatePtr update() override;

	Index index() const override { return _index; }
private:
	virtual void onUpdate() = 0;

	Transitions _transitions;

	Index _index;
};

} // namespace starTopologyEmulator