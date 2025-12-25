#include "StarHub.h"

namespace starTopologyEmulator
{

StarHub::StarHub(
        std::function<void(Timestamp, std::shared_ptr<IMessage>)> sendFunc,
        const std::vector<StationID>& terminalIDs,
        std::unique_ptr<IStarHubStrategy> strategy)
        : _sendFunc(std::move(sendFunc))
        , _terminalIDs(terminalIDs)
        , _strategy(std::move(strategy))
        , _totalSlots(0)
        , _totalAttempts(0)
        , _totalLost(0)
        , _totalSuccess(0)
        , _totalCollisions(0)
{
}

void StarHub::update(Timestamp currentTime)
{
        auto it = _pending.find(currentTime);
        if (it == _pending.end())
        {
                ++_totalSlots;
        }
        else
        {
                std::vector<std::shared_ptr<IMessage>> msgs = std::move(it->second);
                _pending.erase(it);

                ++_totalSlots;
                std::size_t attempts = msgs.size();
                _totalAttempts += attempts;

                if (attempts == 1)
                {
                        // успех
                        ++_totalSuccess;
                        auto raMsg = std::static_pointer_cast<StarStationMessage>(msgs[0]);
                        int stId = raMsg->stationID();

                        auto ack = std::make_shared<StarHubAccessMessage>(stId);
                        _sendFunc(currentTime, ack);
                }
                else
                {
                        ++_totalCollisions;
                        _totalLost += attempts;
                }
        }
        if (_strategy)
        {
                double g = incomeLoad();
                double plr = incomePlr();

                StarHubPlanMessage plan = _strategy->generate(g, plr);
                auto planPtr = std::make_shared<StarHubPlanMessage>(plan);

                // широковещательно всем станциям
                _sendFunc(currentTime, planPtr);
        }
}

void StarHub::handleMessage(std::shared_ptr<IMessage> msg, Timestamp timestamp)
{
        _pending[timestamp].push_back(std::move(msg));
}

double StarHub::incomeLoad() const
{
        return (_totalSlots > 0)
                ? static_cast<double>(_totalAttempts) / _totalSlots
                : 0.0;
}

double StarHub::incomePlr() const
{
        return (_totalAttempts > 0)
                ? static_cast<double>(_totalLost) / _totalAttempts
                : 0.0;
}

std::uint64_t StarHub::getTotalSuccess() const
{
        return _totalSuccess;
}

std::uint64_t StarHub::getTotalCollisions() const
{
        return _totalCollisions;
}

} // namespace starTopologyEmulator

