#include "StarTopologyEmulator/CollisionResolver/CrdsaFrameModel.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <numeric>

namespace starTopologyEmulator
{

CrdsaFrameModel::CrdsaFrameModel(std::shared_ptr<IPhysicalLink> physicalLink, CrdsaSicConfig config)
	: _physicalLink(std::move(physicalLink))
	, _config(config)
	, _rng(config.seed)
{ }

double CrdsaFrameModel::noiseVariance() const
{
	const double esN0Linear = std::pow(10.0, _config.esN0Db / 10.0);
	return esN0Linear > 0.0 ? 1.0 / esN0Linear : 0.0;
}

FrameResult CrdsaFrameModel::process(std::size_t packetCount)
{
	const auto plan = randomPlan(packetCount);
	return resolvePlan(plan, noiseVariance());
}

FrameResult CrdsaFrameModel::resolvePlan(const CrdsaFramePlan& plan, double noiseVariance)
{
	auto slots = buildSlots(plan);
	if (noiseVariance > 0.0)
		addNoise(slots, noiseVariance);
	return runSic(std::move(slots), plan, noiseVariance);
}

CrdsaFramePlan CrdsaFrameModel::randomPlan(std::size_t packetCount)
{
	CrdsaFramePlan plan;
	plan.slotCount = _config.slotsPerFrame;
	plan.packets.resize(packetCount);

	const std::size_t infoBits = _physicalLink->infoBitCount();
	const std::size_t replicaCount = std::min<std::size_t>(_config.replicaCount, plan.slotCount);

	std::uniform_int_distribution<int> bitDist(0, 1);
	std::uniform_real_distribution<double> imbalanceDist(
		-_config.powerImbalanceDb / 2.0, _config.powerImbalanceDb / 2.0);

	std::vector<std::size_t> slotPool(plan.slotCount);
	std::iota(slotPool.begin(), slotPool.end(), std::size_t{ 0 });

	for (auto& packet : plan.packets)
	{
		packet.payload.resize(infoBits);
		for (auto& bit : packet.payload)
			bit = static_cast<IPhysicalLink::Bit>(bitDist(_rng));

		const double amplitudeDb = _config.powerImbalanceDb > 0.0 ? imbalanceDist(_rng) : 0.0;
		packet.amplitude = static_cast<float>(std::pow(10.0, amplitudeDb / 20.0));

		for (std::size_t i = 0; i < replicaCount; ++i)
		{
			std::uniform_int_distribution<std::size_t> pick(i, plan.slotCount - 1);
			std::swap(slotPool[i], slotPool[pick(_rng)]);
		}
		packet.replicaSlots.assign(slotPool.begin(), slotPool.begin() + replicaCount);
	}

	return plan;
}

std::vector<IPhysicalLink::SymbolVector> CrdsaFrameModel::buildSlots(const CrdsaFramePlan& plan) const
{
	const std::size_t symbolCount = _physicalLink->symbolCount();
	std::vector<IPhysicalLink::SymbolVector> slots(
		plan.slotCount, IPhysicalLink::SymbolVector(symbolCount, IPhysicalLink::Symbol{ 0.0f, 0.0f }));

	for (const auto& packet : plan.packets)
	{
		const auto contribution = _physicalLink->modulate(
			packet.payload, IPhysicalLink::Symbol{ packet.amplitude, 0.0f });
		for (const std::size_t slot : packet.replicaSlots)
			for (std::size_t k = 0; k < contribution.size() && k < slots[slot].size(); ++k)
				slots[slot][k] += contribution[k];
	}

	return slots;
}

void CrdsaFrameModel::addNoise(std::vector<IPhysicalLink::SymbolVector>& slots, double noiseVariance)
{
	std::normal_distribution<float> noise(0.0f, static_cast<float>(std::sqrt(noiseVariance / 2.0)));
	for (auto& slot : slots)
		for (auto& symbol : slot)
			symbol += IPhysicalLink::Symbol{ noise(_rng), noise(_rng) };
}

FrameResult CrdsaFrameModel::runSic(
	std::vector<IPhysicalLink::SymbolVector> slots,
	const CrdsaFramePlan& plan,
	double noiseVariance) const
{
	FrameResult result;
	result.packetCount = plan.packets.size();
	result.slotCount = plan.slotCount;
	result.recovered.assign(plan.packets.size(), 0);

	std::map<IPhysicalLink::BitVector, std::size_t> packetByPayload;
	for (std::size_t i = 0; i < plan.packets.size(); ++i)
		packetByPayload.emplace(plan.packets[i].payload, i);

	std::vector<int> remainingInSlot(plan.slotCount, 0);
	for (const auto& packet : plan.packets)
		for (const std::size_t slot : packet.replicaSlots)
			++remainingInSlot[slot];
	for (const int count : remainingInSlot)
		if (count == 1)
			++result.initialSingletonSlots;

	const float noise = static_cast<float>(noiseVariance);

	for (std::uint32_t iteration = 0; iteration < _config.maxSicIterations; ++iteration)
	{
		std::size_t recoveredThisIteration = 0;

		for (std::size_t m = 0; m < plan.slotCount; ++m)
		{
			if (remainingInSlot[m] == 0)
				continue;

			const auto decoded = _physicalLink->demodulateDecode(slots[m], noise);
			if (!decoded.crcOk)
				continue;

			const auto found = packetByPayload.find(decoded.payload);
			if (found == packetByPayload.end())
				continue;

			const std::size_t packetIndex = found->second;
			if (result.recovered[packetIndex])
				continue;

			const auto& replicaSlots = plan.packets[packetIndex].replicaSlots;
			if (std::find(replicaSlots.begin(), replicaSlots.end(), m) == replicaSlots.end())
				continue;

			result.recovered[packetIndex] = 1;
			++result.recoveredCount;
			++recoveredThisIteration;

			const auto reconstruction = _physicalLink->modulate(decoded.payload, decoded.estimatedGain);
			for (const std::size_t slot : replicaSlots)
			{
				for (std::size_t k = 0; k < reconstruction.size() && k < slots[slot].size(); ++k)
					slots[slot][k] -= reconstruction[k];
				--remainingInSlot[slot];
				++result.cancellationCount;
			}
		}

		result.recoveredPerIteration.push_back(recoveredThisIteration);
		result.iterationsUsed = iteration + 1;
		if (recoveredThisIteration == 0)
			break;
	}

	result.offeredLoad = plan.slotCount > 0
		? static_cast<double>(plan.packets.size()) / static_cast<double>(plan.slotCount)
		: 0.0;
	result.throughput = plan.slotCount > 0
		? static_cast<double>(result.recoveredCount) / static_cast<double>(plan.slotCount)
		: 0.0;

	return result;
}

} // namespace starTopologyEmulator
