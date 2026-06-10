#include <gtest/gtest.h>

#include "StarTopologyEmulator/Phy/PhysicalLinkFactory.h"
#include "StarTopologyEmulator/Phy/StubPhysicalLinkConfig.h"

using namespace starTopologyEmulator;
using Symbol = IPhysicalLink::Symbol;
using BitVector = IPhysicalLink::BitVector;
using SymbolVector = IPhysicalLink::SymbolVector;

namespace
{

StubPhysicalLinkConfig makeConfig(std::size_t bits = 4, float threshold = 4.0f)
{
	StubPhysicalLinkConfig config;
	config.infoBits = bits;
	config.decodingSinrThreshold = threshold;
	return config;
}

SymbolVector superpose(const SymbolVector& a, const SymbolVector& b)
{
	SymbolVector sum(a.size());
	for (std::size_t i = 0; i < a.size(); ++i)
		sum[i] = a[i] + b[i];
	return sum;
}

} // namespace

TEST(StubPhysicalLink, ReportsConfiguredSizes)
{
	auto link = PhysicalLinkFactory::makeStub(makeConfig(64));
	EXPECT_EQ(link->infoBitCount(), 64u);
	EXPECT_EQ(link->symbolCount(), 64u);
}

TEST(StubPhysicalLink, ModulatesBpskScaledByGain)
{
	auto link = PhysicalLinkFactory::makeStub(makeConfig());
	const auto symbols = link->modulate(BitVector{ 1, 0, 1, 0 }, Symbol{ 2.0f, 0.0f });

	ASSERT_EQ(symbols.size(), 4u);
	EXPECT_EQ(symbols[0], Symbol(2.0f, 0.0f));
	EXPECT_EQ(symbols[1], Symbol(-2.0f, 0.0f));
	EXPECT_EQ(symbols[2], Symbol(2.0f, 0.0f));
	EXPECT_EQ(symbols[3], Symbol(-2.0f, 0.0f));
}

TEST(StubPhysicalLink, NoiselessRoundTripRecoversBits)
{
	auto link = PhysicalLinkFactory::makeStub(makeConfig());
	const BitVector bits{ 1, 1, 0, 1 };

	const auto tx = link->modulate(bits, Symbol{ 1.0f, 0.0f });
	const auto rx = link->demodulateDecode(tx, 0.0f);

	EXPECT_TRUE(rx.crcOk);
	EXPECT_EQ(rx.payload, bits);
	EXPECT_NEAR(rx.estimatedGain.real(), 1.0f, 1e-5f);
	EXPECT_NEAR(rx.estimatedGain.imag(), 0.0f, 1e-5f);
}

TEST(StubPhysicalLink, ModulationIsLinearForCancellation)
{
	auto link = PhysicalLinkFactory::makeStub(makeConfig());
	const auto a = link->modulate(BitVector{ 1, 1, 1, 1 }, Symbol{ 4.0f, 0.0f });
	const auto b = link->modulate(BitVector{ 1, 0, 1, 0 }, Symbol{ 1.0f, 0.0f });

	const auto sum = superpose(a, b);
	for (std::size_t i = 0; i < sum.size(); ++i)
		EXPECT_EQ(sum[i] - a[i], b[i]);
}

TEST(StubPhysicalLink, DominantSignalCapturesOverWeakInterferer)
{
	auto link = PhysicalLinkFactory::makeStub(makeConfig(4, 4.0f));
	const BitVector dominantBits{ 1, 1, 1, 1 };
	const auto dominant = link->modulate(dominantBits, Symbol{ 4.0f, 0.0f });
	const auto weak = link->modulate(BitVector{ 1, 0, 1, 0 }, Symbol{ 1.0f, 0.0f });

	const auto rx = link->demodulateDecode(superpose(dominant, weak), 0.0f);

	EXPECT_TRUE(rx.crcOk);
	EXPECT_EQ(rx.payload, dominantBits);
	EXPECT_NEAR(rx.estimatedGain.real(), 4.0f, 1e-5f);
}

TEST(StubPhysicalLink, BalancedCollisionFailsToDecode)
{
	auto link = PhysicalLinkFactory::makeStub(makeConfig(4, 4.0f));
	const auto a = link->modulate(BitVector{ 1, 1, 1, 1 }, Symbol{ 1.0f, 0.0f });
	const auto b = link->modulate(BitVector{ 1, 0, 1, 0 }, Symbol{ 1.0f, 0.0f });

	EXPECT_FALSE(link->demodulateDecode(superpose(a, b), 0.0f).crcOk);
}

TEST(StubPhysicalLink, CancellingDominantRevealsWeakSignal)
{
	auto link = PhysicalLinkFactory::makeStub(makeConfig(4, 4.0f));
	const BitVector weakBits{ 1, 0, 1, 0 };
	const auto dominant = link->modulate(BitVector{ 1, 1, 1, 1 }, Symbol{ 4.0f, 0.0f });
	const auto weak = link->modulate(weakBits, Symbol{ 1.0f, 0.0f });
	const auto rx = superpose(dominant, weak);

	const auto first = link->demodulateDecode(rx, 0.0f);
	ASSERT_TRUE(first.crcOk);

	const auto reconstructed = link->modulate(first.payload, first.estimatedGain);
	SymbolVector residual(rx.size());
	for (std::size_t i = 0; i < rx.size(); ++i)
		residual[i] = rx[i] - reconstructed[i];

	const auto second = link->demodulateDecode(residual, 0.0f);
	EXPECT_TRUE(second.crcOk);
	EXPECT_EQ(second.payload, weakBits);
}
