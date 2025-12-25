#pragma once

#include <cstdint>

#include "StarTopologyEmulator/IFaces/IMessage.h"
#include "StarTopologyEmulator/StarTopologyEmulatorExport.h"

namespace starTopologyEmulator
{

class STAR_TOPOLOGY_EMULATOR_LIB_EXPORT StarHubPlanMessage : public IMessage
{
public:
        struct BackoffConfig
        {
                std::uint8_t baseWindow = 1;
                std::uint8_t maxWindow = 32;
                double exponentBase = 2.0;
                bool   useExponential = true;
                double pTx = 1.0;
        };
public:
    StarHubPlanMessage(
        std::uint8_t onlineSlots,
        std::uint8_t yellowSlots,
        std::uint8_t raSlots,
        const BackoffConfig& cfg)
        : _onlineSlotsCountInFrame(onlineSlots)
        , _yellowSlotsCountInFrame(yellowSlots)
        , _randomAccessSlotsCountInFrame(raSlots)
        , _backoff(cfg)
    {}

    MessageType type() const override { return MessageType::StarHubPlan; }

    std::uint8_t onlineSlotsCountInFrame() { return _onlineSlotsCountInFrame; }
    std::uint8_t yellowSlotsCountInFrame() { return _yellowSlotsCountInFrame; }
    std::uint8_t randomAccessSlotsCountInFrame() const { return _randomAccessSlotsCountInFrame; }
    BackoffConfig backoff() const { return _backoff; }
private:
    std::uint8_t _onlineSlotsCountInFrame;
    std::uint8_t _yellowSlotsCountInFrame;
    std::uint8_t _randomAccessSlotsCountInFrame;
    BackoffConfig _backoff;
};

} // namespace starTopologyEmulator
