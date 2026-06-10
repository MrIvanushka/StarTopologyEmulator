#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <random>
#include <vector>

#include "StarTopologyEmulator/CollisionResolver/CrdsaSicConfig.h"
#include "StarTopologyEmulator/IFaces/IPhysicalLink.h"
#include "StarTopologyEmulator/StarTopologyEmulatorExport.h"

namespace starTopologyEmulator
{

struct STAR_TOPOLOGY_EMULATOR_LIB_EXPORT CrdsaFramePlan
{
	struct Packet
	{
		std::vector<std::size_t> replicaSlots;
		float amplitude = 1.0f;
		IPhysicalLink::BitVector payload;
	};

	std::size_t slotCount = 0;
	std::vector<Packet> packets;
};

struct STAR_TOPOLOGY_EMULATOR_LIB_EXPORT FrameResult
{
	std::size_t packetCount = 0;
	std::size_t slotCount = 0;
	std::size_t recoveredCount = 0;
	std::vector<std::uint8_t> recovered;
	std::uint32_t iterationsUsed = 0;
	std::vector<std::size_t> recoveredPerIteration;
	std::size_t initialSingletonSlots = 0;
	std::size_t cancellationCount = 0;
	double offeredLoad = 0.0;
	double throughput = 0.0;
};

class STAR_TOPOLOGY_EMULATOR_LIB_EXPORT CrdsaFrameModel
{
public:
	CrdsaFrameModel(std::shared_ptr<IPhysicalLink> physicalLink, CrdsaSicConfig config);

	FrameResult process(std::size_t packetCount);

	FrameResult resolvePlan(const CrdsaFramePlan& plan, double noiseVariance);

	double noiseVariance() const;

private:
	CrdsaFramePlan randomPlan(std::size_t packetCount);

	std::vector<IPhysicalLink::SymbolVector> buildSlots(const CrdsaFramePlan& plan) const;

	void addNoise(std::vector<IPhysicalLink::SymbolVector>& slots, double noiseVariance);

	FrameResult runSic(
		std::vector<IPhysicalLink::SymbolVector> slots,
		const CrdsaFramePlan& plan,
		double noiseVariance) const;

	std::shared_ptr<IPhysicalLink> _physicalLink;
	CrdsaSicConfig _config;
	std::mt19937_64 _rng;
};

} // namespace starTopologyEmulator
