#include "StarHub.h"

#include <cmath>

#include "StarTopologyEmulator/Messages/BacklogReportMessage.h"

namespace starTopologyEmulator
{

StarHub::StarHub(
	std::function<void(Timestamp, std::shared_ptr<IMessage>)> sendFunc,
	std::shared_ptr<IIncomeLoadEstimator> incomeLoadEstimator,
	std::shared_ptr<IFrameCalculator> frameCalculator,
	std::shared_ptr<IDynamicFrameSettings> dynamicFrameSettings,
	std::unique_ptr<IStarHubStrategy> strategy,
	std::shared_ptr<IBacklogAccumulator> backlogAccumulator,
	Timestamp tts,
	MetricScope scope)
	: _sendFunc(std::move(sendFunc))
	, _incomeLoadEstimator(incomeLoadEstimator)
	, _frameCalculator(frameCalculator)
	, _dynamicFrameSettings(dynamicFrameSettings)
	, _strategy(std::move(strategy))
	, _tts(tts)
	, _backlogAccumulator(std::move(backlogAccumulator))
	, _scope(std::move(scope))
{
	if (_scope.active())
	{
		_hPendingAnswers = _scope.registerMetric("Очередь ответов станциям");
		_hDaThroughput = _scope.registerMetric("Пропускная способность DA-сегмента");
	}
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
	if (msg->type() == MessageType::BacklogReport)
	{
		_backlogAccumulator->handleReport(
			std::static_pointer_cast<BacklogReportMessage>(msg));
		return;
	}

	FrameMoment handleMoment = _frameCalculator->frameMoment(arrivalTime);

	auto currentPlan = _dynamicFrameSettings->currentPlan(handleMoment.frameNumber);
	if (!currentPlan)
		return;

	if (handleMoment.slotNumber < currentPlan->randomAccessSlotsCountInFrame()) {

		if (msg->type() == MessageType::StarStation)
		{
			_pendingAnswers.push_back(std::dynamic_pointer_cast<StarStationMessage>(msg)->stationID());
			_frameAccumulator.successSlots++;
		}
		else if (msg->type() == MessageType::CollisionReport) {
			_frameAccumulator.collisionSlots++;
		}
		_frameAccumulator.idleSlots -= 1;
	}
}

Timestamp StarHub::tts() const
{
	return _tts;
}

void StarHub::onFrameEnd(std::uint64_t frameNumber)
{
	auto currentPlan = _dynamicFrameSettings->currentPlan(frameNumber);

	_frameAccumulator.totalRaSlots = currentPlan ? currentPlan->randomAccessSlotsCountInFrame() : 0;
	_frameAccumulator.frame = frameNumber;

	_incomeLoadEstimator->update(_frameAccumulator);
	auto targetFrameNumber = frameNumber + 2 + _tts * 5 / (_frameCalculator->frameConfig().slotDuration * _frameCalculator->frameConfig().slotCountInFrame);
	auto newPlan = _strategy->generate(frameNumber, targetFrameNumber);

	const double estTotalTransmitting = _incomeLoadEstimator->incomeLoad()
		* static_cast<double>(_frameAccumulator.totalRaSlots);
	const double estCollidedRaw = estTotalTransmitting
		- static_cast<double>(_frameAccumulator.successSlots);
	const std::uint32_t estCollided = estCollidedRaw > 0.0
		? static_cast<std::uint32_t>(std::lround(estCollidedRaw))
		: 0u;
	newPlan->setCollidedStationCount(estCollided);

	_dynamicFrameSettings->handlePlan(newPlan);

	const Timestamp broadcastTime = _frameCalculator->slotBeginTime(frameNumber + 1, 0);
	_sendFunc(broadcastTime, newPlan);

	auto operationPlan = _backlogAccumulator->generateOperationPlan(
		targetFrameNumber,
		newPlan->onlineSlotsCountInFrame());
	_sendFunc(broadcastTime, operationPlan);

	_frameAccumulator = RandomAccessFrameResult();
	auto nextFramePlan = _dynamicFrameSettings->currentPlan(frameNumber + 1);
	_frameAccumulator.idleSlots = nextFramePlan ? nextFramePlan->randomAccessSlotsCountInFrame() : 0;

	sendAnswersToStations(broadcastTime);

	_scope.emit(_hPendingAnswers, frameNumber, static_cast<double>(_pendingAnswers.size()));

	const auto& frameCfg = _frameCalculator->frameConfig();
	const Timestamp frameDuration = frameCfg.slotDuration * static_cast<Timestamp>(frameCfg.slotCountInFrame);
	const auto daSlots = currentPlan ? currentPlan->onlineSlotsCountInFrame() : 0u;
	const double daThroughput = frameDuration > 0
		? static_cast<double>(daSlots) * static_cast<double>(frameCfg.bitsPerSlot) / static_cast<double>(frameDuration)
		: 0.0;
	_scope.emit(_hDaThroughput, frameNumber, daThroughput);
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
