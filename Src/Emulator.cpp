#include "Emulator.h"

#include "StarTopologyEmulator/Messages/StarHubAccessMessage.h"

namespace starTopologyEmulator
{

Emulator::Emulator(
        std::function<std::shared_ptr<IStarStation>(SendFunc, StationID, RTT)> stationFactory,
        std::function<std::shared_ptr<IStarHub>(SendFunc, const std::vector<StationID>&)> hubFactory,
        int stationCount, int rttSlots)
    : _rttSlots(rttSlots)
{
        std::vector<StationID> abonentIDs(stationCount);
        _stations.resize(stationCount);

        for (auto i = 0u; i < stationCount; ++i)
        {
                _stations[i] = stationFactory(makeStationSendFunc(i), i, _rttSlots);
                abonentIDs[i] = i;
        }

        _hub = hubFactory(makeHubSendFunc(), abonentIDs);
}

std::function<void(Emulator::Timestamp, std::shared_ptr<IMessage>)> Emulator::makeHubSendFunc()
{
    return [this](Timestamp t, std::shared_ptr<IMessage> msg)
        {
            enqueueFromHub(t, std::move(msg));
        };
}

std::function<void(Emulator::Timestamp, std::shared_ptr<IMessage>)> Emulator::makeStationSendFunc(int stationID)
{
    return [this, stationID](Timestamp t, std::shared_ptr<IMessage> msg)
        {
            enqueueFromStation(stationID, t, std::move(msg));
        };
}

std::shared_ptr<IStarHub> Emulator::hub() const
{
    return _hub;
}

const std::vector<std::shared_ptr<IStarStation>>& Emulator::stations() const
{
    return _stations;
}

void Emulator::update(Timestamp currentTime)
{
    auto range = _queue.equal_range(currentTime);
    for (auto it = range.first; it != range.second; ++it)
    {
        const QueuedMessage& qm = it->second;
        if (qm.direction == Direction::ToHub)
        {
            if (_hub)
                _hub->handleMessage(qm.msg, currentTime);
        }
        else
        {
            for (auto& st : _stations)
            {
                if (st->id() == qm.stationID)
                {
                    st->handleMessage(qm.msg, currentTime);
                    break;
                }
            }
        }
    }
    _queue.erase(range.first, range.second);

    for (auto& st : _stations)
    {
        TerminalState s = st->currentState();
        _stationStats[st->id()][s]++;
    }
}

const std::map<StationID, Emulator::StateCounter>& Emulator::stationStats() const
{
    return _stationStats;
}

void Emulator::enqueueFromHub(Timestamp sendTime, std::shared_ptr<IMessage> msg)
{
    if (!msg) return;

    if (msg->type() == MessageType::StarHubPlan)
    {
        for (auto& st : _stations)
        {
            QueuedMessage qm;
            qm.deliveryTime = sendTime + _rttSlots;
            qm.direction = Direction::ToStation;
            qm.stationID = st->id();
            qm.msg = msg;
            _queue.emplace(qm.deliveryTime, qm);
        }
    }
    else if (msg->type() == MessageType::StarHubAccess)
    {
        auto ack = std::static_pointer_cast<StarHubAccessMessage>(msg);
        QueuedMessage qm;
        qm.deliveryTime = sendTime + _rttSlots;
        qm.direction = Direction::ToStation;
        qm.stationID = ack->stationID();
        qm.msg = msg;
        _queue.emplace(qm.deliveryTime, qm);
    }
}

void Emulator::enqueueFromStation(
    int stationID,
    Timestamp sendTime,
    std::shared_ptr<IMessage> msg)
{
    if (!msg) return;

    QueuedMessage qm;
    qm.deliveryTime = sendTime + _rttSlots;
    qm.direction = Direction::ToHub;
    qm.stationID = stationID;
    qm.msg = msg;
    _queue.emplace(qm.deliveryTime, qm);
}

} // namespace starTopologyEmulator
