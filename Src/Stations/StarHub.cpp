#include "StarHub.h"

namespace starTopologyEmulator
{

StarHub::StarHub(
	std::function<void(Timestamp, std::shared_ptr<IMessage>)> sendFunc,
	std::unique_ptr<IIncomeLoadEstimator> incomeLoadEstimator,
	std::unique_ptr<IFrameCalculator> frameCalculator,
	std::unique_ptr<IStarHubStrategy> strategy,
	Timestamp tts)
	: _sendFunc(std::move(sendFunc))
	, _incomeLoadEstimator(std::move(incomeLoadEstimator))
	, _frameCalculator(std::move(frameCalculator))
	, _strategy(std::move(strategy))
	, _tts(tts)
{
	REGISTER_METRIC_SUBFOLDER(_strategy.get());
	REGISTER_METRIC_SUBFOLDER(_incomeLoadEstimator.get());
}

void StarHub::update(Timestamp currentTime)
{
	FrameMoment moment = _frameCalculator->frameMoment(currentTime);

	if (moment.frameNumber > _lastProcessedFrame)
	{
		onFrameEnd(_lastProcessedFrame);
		_lastProcessedFrame = moment.frameNumber;
	}
}

void StarHub::handleMessage(std::shared_ptr<IMessage> msg, Timestamp arrivalTime)
{
	FrameMoment handleMoment = _frameCalculator->frameMoment(arrivalTime);

	if (handleMoment.frameNumber != _lastProcessedFrame)
		return;

	if (handleMoment.slotNumber < _currentPlan->randomAccessSlotsCountInFrame()) {

		if (msg->type() == MessageType::StarStation) {
			_frameAccumulator.successSlots++;
		}
		else if (msg->type() == MessageType::CollisionReport) {
			_frameAccumulator.collisionSlots++;
		}
	}
}

Timestamp StarHub::tts() const
{
	return _tts;
}

void StarHub::onFrameEnd(std::uint64_t frameNumber)
{
	_frameAccumulator.totalRaSlots = _currentPlan->randomAccessSlotsCountInFrame();

	_incomeLoadEstimator->update(_frameAccumulator);

	auto g = _incomeLoadEstimator->incomeLoad();
	auto plr = _incomeLoadEstimator->plr();

	_currentPlan = _strategy->generate(g, plr);

	_sendFunc(_frameCalculator->slotBeginTime(frameNumber + 1, 0), _currentPlan);
	_frameAccumulator = RandomAccessFrameResult();
}

void StarHub::sendAnswersToStations(Timestamp sendTime)
{
	for (auto id : _pendingAnswers)
	{
		_sendFunc(sendTime, std::make_shared<StarHubAccessMessage>(id));
	}
	_pendingAnswers.clear();
}

} // namespace starTopologyEmulator
