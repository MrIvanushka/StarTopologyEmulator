#pragma once

#include <cstdint>

#include "StarTopologyEmulator/IFaces/ITerminal.h"

namespace starTopologyEmulator
{

class IStarHub : public ITerminal
{
public:
    virtual double incomeLoad() const = 0;

    virtual double incomePlr() const = 0;

    virtual std::uint64_t getTotalSuccess() const = 0;

    virtual std::uint64_t getTotalCollisions() const = 0;
};

} // namespace starTopologyEmulator

