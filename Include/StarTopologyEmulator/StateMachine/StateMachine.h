#pragma once

#include <memory>
#include <vector>

#include "StarTopologyEmulator/StarTopologyEmulatorExport.h"
#include "StarTopologyEmulator/StateMachine/IFaces/IState.h"

namespace starTopologyEmulator
{

class STAR_TOPOLOGY_EMULATOR_LIB_EXPORT StateMachine
{
private:
	typedef std::vector<std::shared_ptr<IState>> StateCollection;
public:
	StateMachine(const StateCollection&);

	void update();

	IState::Index currentStateIndex() const;
private:
	StateCollection _states;
	IState* _currentState;
};

} // namespace starTopologyEmulator
