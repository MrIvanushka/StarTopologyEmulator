#ifdef STE_WITH_AFF3CT

#include <memory>
#include <vector>

#include <gtest/gtest.h>

#include "StarTopologyEmulator/Phy/Aff3ctPhysicalLinkConfig.h"
#include "StarTopologyEmulator/Phy/PhysicalLinkFactory.h"

using namespace starTopologyEmulator;

namespace
{

Aff3ctPhysicalLinkConfig ldpcConfig()
{
	Aff3ctPhysicalLinkConfig config;
	config.hMatrixPath = STE_AFF3CT_TEST_MATRIX;
	config.infoBits = 4080;
	config.crcSize = 16;
	config.crcPolynomial = "16-CCITT";
	config.bpIterations = 50;
	return config;
}

} // namespace

class Aff3ctPhysicalLinkTest : public ::testing::Test
{
protected:
	static void SetUpTestSuite()
	{
		if (!_link)
			_link = PhysicalLinkFactory::makeAff3ctLdpc(ldpcConfig());
	}

	static void TearDownTestSuite()
	{
		_link.reset();
	}

	static std::unique_ptr<IPhysicalLink> _link;
};

std::unique_ptr<IPhysicalLink> Aff3ctPhysicalLinkTest::_link = nullptr;

TEST_F(Aff3ctPhysicalLinkTest, ReportsMatrixDimensions)
{
	EXPECT_EQ(_link->infoBitCount(), 4080u);
	EXPECT_EQ(_link->symbolCount(), 10240u);
}

TEST_F(Aff3ctPhysicalLinkTest, ModulationScalesWithGain)
{
	IPhysicalLink::BitVector payload(4080, IPhysicalLink::Bit{ 0 });
	const auto unit = _link->modulate(payload, IPhysicalLink::Symbol{ 1.0f, 0.0f });
	const auto scaled = _link->modulate(payload, IPhysicalLink::Symbol{ 3.0f, 0.0f });

	ASSERT_EQ(unit.size(), scaled.size());
	for (std::size_t i = 0; i < unit.size(); ++i)
		EXPECT_NEAR(scaled[i].real(), 3.0f * unit[i].real(), 1e-4f);
}

TEST_F(Aff3ctPhysicalLinkTest, CleanRoundTripRecoversPayload)
{
	IPhysicalLink::BitVector payload(4080);
	for (std::size_t i = 0; i < payload.size(); ++i)
		payload[i] = static_cast<IPhysicalLink::Bit>(i % 2);

	const auto tx = _link->modulate(payload, IPhysicalLink::Symbol{ 1.0f, 0.0f });
	ASSERT_EQ(tx.size(), 10240u);

	const auto rx = _link->demodulateDecode(tx, 0.1f);

	EXPECT_TRUE(rx.crcOk);
	EXPECT_EQ(rx.payload, payload);
	EXPECT_NEAR(rx.estimatedGain.real(), 1.0f, 0.1f);
}

#endif // STE_WITH_AFF3CT
