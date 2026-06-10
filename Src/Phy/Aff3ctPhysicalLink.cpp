#ifdef STE_WITH_AFF3CT

#include "Aff3ctPhysicalLink.h"

#include <algorithm>
#include <cmath>
#include <complex>
#include <numeric>
#include <stdexcept>
#include <vector>

#include <aff3ct.hpp>

namespace starTopologyEmulator
{

struct Aff3ctPhysicalLink::Impl
{
	std::size_t infoBits = 0;
	std::size_t ldpcInfoBits = 0;
	std::size_t codewordBits = 0;

	aff3ct::tools::Sparse_matrix H;
	std::unique_ptr<aff3ct::module::CRC_polynomial<int>> crc;
	std::unique_ptr<aff3ct::module::Encoder_LDPC<int>> encoder;
	std::unique_ptr<aff3ct::module::Decoder_LDPC_BP_flooding_SPA<int, float>> decoder;

	explicit Impl(const Aff3ctPhysicalLinkConfig& config)
	{
		int matrixRows = 0;
		int matrixCols = 0;
		aff3ct::tools::LDPC_matrix_handler::read_matrix_size(config.hMatrixPath, matrixRows, matrixCols);

		H = aff3ct::tools::LDPC_matrix_handler::read(config.hMatrixPath);

		infoBits = config.infoBits;
		ldpcInfoBits = config.infoBits + config.crcSize;
		codewordBits = static_cast<std::size_t>(matrixCols);

		const std::size_t matrixInfoBits = static_cast<std::size_t>(matrixCols - matrixRows);
		if (ldpcInfoBits != matrixInfoBits)
			throw std::runtime_error(
				"Aff3ctPhysicalLink: infoBits + crcSize (" + std::to_string(ldpcInfoBits)
				+ ") does not match the matrix information length (" + std::to_string(matrixInfoBits) + ")");

		crc = std::make_unique<aff3ct::module::CRC_polynomial<int>>(
			static_cast<int>(infoBits), config.crcPolynomial, static_cast<int>(config.crcSize));

		const auto format = aff3ct::tools::LDPC_matrix_handler::get_matrix_format(config.hMatrixPath);
		if (format == aff3ct::tools::LDPC_matrix_handler::Matrix_format::QC)
			encoder = std::make_unique<aff3ct::module::Encoder_LDPC_from_QC<int>>(
				static_cast<int>(ldpcInfoBits), static_cast<int>(codewordBits), H);
		else
			encoder = std::make_unique<aff3ct::module::Encoder_LDPC_from_H<int>>(
				static_cast<int>(ldpcInfoBits), static_cast<int>(codewordBits), H);

		auto infoBitsPos = encoder->get_info_bits_pos();
		if (infoBitsPos.empty())
		{
			infoBitsPos.resize(ldpcInfoBits);
			std::iota(infoBitsPos.begin(), infoBitsPos.end(), 0u);
		}

		decoder = std::make_unique<aff3ct::module::Decoder_LDPC_BP_flooding_SPA<int, float>>(
			static_cast<int>(ldpcInfoBits), static_cast<int>(codewordBits),
			static_cast<int>(config.bpIterations), H, infoBitsPos);
	}
};

Aff3ctPhysicalLink::Aff3ctPhysicalLink(Aff3ctPhysicalLinkConfig config)
	: _impl(std::make_unique<Impl>(config))
{ }

Aff3ctPhysicalLink::~Aff3ctPhysicalLink() = default;

std::size_t Aff3ctPhysicalLink::infoBitCount() const
{
	return _impl->infoBits;
}

std::size_t Aff3ctPhysicalLink::symbolCount() const
{
	return _impl->codewordBits;
}

IPhysicalLink::SymbolVector Aff3ctPhysicalLink::modulate(const BitVector& payload, Symbol gain) const
{
	std::vector<int> info(payload.begin(), payload.end());
	std::vector<int> infoWithCrc(_impl->ldpcInfoBits);
	_impl->crc->build(info, infoWithCrc);

	std::vector<int> codeword(_impl->codewordBits);
	_impl->encoder->encode(infoWithCrc, codeword);

	SymbolVector symbols(_impl->codewordBits);
	for (std::size_t i = 0; i < codeword.size(); ++i)
		symbols[i] = gain * (codeword[i] == 0 ? 1.0f : -1.0f);

	return symbols;
}

IPhysicalLink::DecodeResult Aff3ctPhysicalLink::demodulateDecode(
	const SymbolVector& received,
	float noiseVariance) const
{
	DecodeResult result;
	if (received.size() != _impl->codewordBits)
		return result;

	const std::size_t n = received.size();
	std::complex<double> correlation{ 0.0, 0.0 };
	for (const Symbol& sample : received)
	{
		const double sign = sample.real() >= 0.0f ? 1.0 : -1.0;
		correlation += std::complex<double>(sample) * sign;
	}
	const std::complex<double> gain = correlation / static_cast<double>(n);
	result.estimatedGain = Symbol{ static_cast<float>(gain.real()), static_cast<float>(gain.imag()) };

	const double sigma2 = std::max(static_cast<double>(noiseVariance) / 2.0, 1e-9);
	const double scale = 2.0 * gain.real() / sigma2;

	std::vector<float> llr(n);
	for (std::size_t i = 0; i < n; ++i)
		llr[i] = static_cast<float>(scale * received[i].real());

	std::vector<int> decoded(_impl->ldpcInfoBits);
	_impl->decoder->decode_siho(llr, decoded);

	result.crcOk = _impl->crc->check(decoded);

	result.payload.resize(_impl->infoBits);
	for (std::size_t i = 0; i < _impl->infoBits; ++i)
		result.payload[i] = static_cast<Bit>(decoded[i]);

	return result;
}

} // namespace starTopologyEmulator

#endif // STE_WITH_AFF3CT
