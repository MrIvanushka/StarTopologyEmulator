#include <initializer_list>
#include <memory>
#include <vector>

#include <gtest/gtest.h>

#include "StarTopologyEmulator/CollisionResolver/CollisionResolverFactory.h"
#include "StarTopologyEmulator/CollisionResolver/CrdsaFrameModel.h"
#include "StarTopologyEmulator/CollisionResolver/CrdsaSicConfig.h"
#include "StarTopologyEmulator/Messages/StarStationMessage.h"
#include "StarTopologyEmulator/Phy/PhysicalLinkFactory.h"

using namespace starTopologyEmulator;

namespace
{

IPhysicalLink::BitVector bits(std::initializer_list<int> values)
{
	IPhysicalLink::BitVector out;
	out.reserve(values.size());
	for (const int value : values)
		out.push_back(static_cast<IPhysicalLink::Bit>(value));
	return out;
}

std::shared_ptr<IPhysicalLink> makeStub(std::size_t infoBits, float threshold = 4.0f)
{
	StubPhysicalLinkConfig config;
	config.infoBits = infoBits;
	config.decodingSinrThreshold = threshold;
	return PhysicalLinkFactory::makeStub(config);
}

CrdsaSicConfig planConfig()
{
	CrdsaSicConfig config;
	config.maxSicIterations = 16;
	return config;
}

} // namespace

TEST(CrdsaFrameModel, SingletonReplicasRecoverPacket)
{
	CrdsaFrameModel model(makeStub(8), planConfig());

	CrdsaFramePlan plan;
	plan.slotCount = 2;
	plan.packets.push_back({ { 0, 1 }, 1.0f, bits({ 0, 0, 0, 0, 0, 0, 0, 0 }) });

	const auto result = model.resolvePlan(plan, 0.0);

	EXPECT_EQ(result.recoveredCount, 1u);
	EXPECT_EQ(result.recovered[0], 1u);
	EXPECT_EQ(result.initialSingletonSlots, 2u);
}

TEST(CrdsaFrameModel, BalancedDoubleCollisionIsUnresolvable)
{
	CrdsaFrameModel model(makeStub(8), planConfig());

	CrdsaFramePlan plan;
	plan.slotCount = 2;
	plan.packets.push_back({ { 0, 1 }, 1.0f, bits({ 0, 0, 0, 0, 0, 0, 0, 0 }) });
	plan.packets.push_back({ { 0, 1 }, 1.0f, bits({ 1, 1, 1, 1, 0, 0, 0, 0 }) });

	const auto result = model.resolvePlan(plan, 0.0);

	EXPECT_EQ(result.recoveredCount, 0u);
	EXPECT_EQ(result.initialSingletonSlots, 0u);
	EXPECT_EQ(result.iterationsUsed, 1u);
}

TEST(CrdsaFrameModel, SicCancellationChainsToResolveBuriedPacket)
{
	CrdsaFrameModel model(makeStub(8), planConfig());

	CrdsaFramePlan plan;
	plan.slotCount = 2;
	plan.packets.push_back({ { 0 }, 1.0f, bits({ 0, 0, 0, 0, 0, 0, 0, 0 }) });
	plan.packets.push_back({ { 0, 1 }, 1.0f, bits({ 1, 1, 1, 1, 0, 0, 0, 0 }) });

	const auto result = model.resolvePlan(plan, 0.0);

	EXPECT_EQ(result.recoveredCount, 2u);
	EXPECT_EQ(result.recovered[0], 1u);
	EXPECT_EQ(result.recovered[1], 1u);
	ASSERT_GE(result.recoveredPerIteration.size(), 2u);
	EXPECT_EQ(result.recoveredPerIteration[0], 1u);
	EXPECT_EQ(result.recoveredPerIteration[1], 1u);
}

TEST(CrdsaFrameModel, StrongPacketIsCapturedThenWeakRevealed)
{
	CrdsaFrameModel model(makeStub(8), planConfig());

	CrdsaFramePlan plan;
	plan.slotCount = 1;
	plan.packets.push_back({ { 0 }, 4.0f, bits({ 1, 1, 1, 1, 1, 1, 1, 1 }) });
	plan.packets.push_back({ { 0 }, 1.0f, bits({ 1, 1, 1, 1, 0, 0, 0, 0 }) });

	const auto result = model.resolvePlan(plan, 0.0);

	EXPECT_EQ(result.recoveredCount, 2u);
	EXPECT_EQ(result.recovered[0], 1u);
	EXPECT_EQ(result.recovered[1], 1u);
}

TEST(CrdsaFrameModel, ProcessRecoversLonePacket)
{
	CrdsaSicConfig config;
	config.slotsPerFrame = 50;
	config.replicaCount = 2;
	config.esN0Db = 20.0;
	config.seed = 7;

	CrdsaFrameModel model(makeStub(64), config);
	const auto result = model.process(1);

	EXPECT_EQ(result.packetCount, 1u);
	EXPECT_EQ(result.recoveredCount, 1u);
	EXPECT_EQ(result.recovered.size(), 1u);
	EXPECT_DOUBLE_EQ(result.offeredLoad, 1.0 / 50.0);
	EXPECT_DOUBLE_EQ(result.throughput, 1.0 / 50.0);
}

TEST(CrdsaFrameModel, ProcessReportsConsistentSizes)
{
	CrdsaSicConfig config;
	config.slotsPerFrame = 50;
	config.replicaCount = 2;
	config.esN0Db = 15.0;
	config.seed = 99;

	CrdsaFrameModel model(makeStub(64), config);
	const auto result = model.process(20);

	EXPECT_EQ(result.packetCount, 20u);
	EXPECT_EQ(result.recovered.size(), 20u);
	EXPECT_LE(result.recoveredCount, 20u);
}

TEST(CrdsaSicCollisionResolver, RecoveredPacketIsDeliveredToHub)
{
	CrdsaSicConfig config;
	config.slotsPerFrame = 50;
	config.replicaCount = 2;
	config.esN0Db = 20.0;
	config.seed = 3;

	auto resolver = CollisionResolverFactory::makeCrdsaSic(makeStub(64), config);

	auto message = std::make_shared<StarStationMessage>(/*id=*/0);
	std::vector<ICollisionResolver::SlotTransmission> transmissions;
	transmissions.push_back({ /*slotTime=*/123, /*slotNumber=*/4, message });

	const auto outcome = resolver->resolveFrame(transmissions);

	EXPECT_EQ(outcome.okCount, 1u);
	EXPECT_EQ(outcome.lostCount, 0u);
	ASSERT_EQ(outcome.toHub.size(), 1u);
	EXPECT_EQ(outcome.toHub[0].msg, message);
	EXPECT_EQ(outcome.toHub[0].deliveryTime, 123);
}

TEST(CrdsaSicCollisionResolver, EmptyFrameYieldsEmptyOutcome)
{
	auto resolver = CollisionResolverFactory::makeCrdsaSic(makeStub(64), CrdsaSicConfig{});

	const auto outcome = resolver->resolveFrame({});

	EXPECT_EQ(outcome.okCount, 0u);
	EXPECT_EQ(outcome.lostCount, 0u);
	EXPECT_TRUE(outcome.toHub.empty());
}
