#pragma once

#include <cstdint>
#include <memory>

#include "StarTopologyEmulator/IFaces/IMessage.h"

namespace starTopologyEmulator
{

class ITerminal
{
public:
    using Timestamp = std::uint64_t;

    virtual ~ITerminal() = default;

    virtual void update(Timestamp) = 0;

    virtual void handleMessage(std::shared_ptr<IMessage>, Timestamp) = 0;
};

} // namespace starTopologyEmulator
