#include "./compress.hpp"
#include "./streams.hpp"
#include <brotli/encode.h>
#include <brotli/decode.h>
#include <array>

using namespace Lambda;


Lambda::Error Compress::brotliCompressBuffer(const std::vector<uint8_t>& input, std::vector<uint8_t>& output) {
	return brotliCompressBuffer(input, output, 6);
}

Lambda::Error Compress::brotliCompressBuffer(const std::vector<uint8_t>& input, std::vector<uint8_t>& output, int quality) {

	if (!input.size()) return { "Empty input buffer" };
	if (output.size()) output.clear();

	BrotliCompressStream br;

	if (quality >= BROTLI_MIN_QUALITY) {
		if (quality > BROTLI_MAX_QUALITY) quality = BROTLI_MAX_QUALITY;
		if (!BrotliEncoderSetParameter(br.stream, BROTLI_PARAM_QUALITY, quality))
			return { "Failed to set compression quality" };
	}
	
	std::array<uint8_t, br.chunk> buffOut;

	size_t slider_in = 0;
	bool eob = false;

	auto next_in = input.data() + slider_in;
	size_t available_in = 0;

	auto next_out = buffOut.data();
	size_t available_out = buffOut.size();

	while (true) {

		if (available_in == 0 && !eob) {
			eob = (slider_in + br.chunk) >= input.size();
			next_in = input.data() + slider_in;
			available_in = eob ? input.size() - slider_in : br.chunk;
			slider_in += available_in;
		}

		if (!BrotliEncoderCompressStream(br.stream, eob ? BROTLI_OPERATION_FINISH : BROTLI_OPERATION_PROCESS, &available_in, &next_in, &available_out, &next_out, nullptr)) {
			throw Lambda::Error("brotli encoder failed");
		}

		if (available_out == 0) {
			output.insert(output.end(), buffOut.begin(), buffOut.end());
			next_out = buffOut.data();
			available_out = buffOut.size();
		}

		if (BrotliEncoderIsFinished(br.stream)) {
			output.insert(output.end(), buffOut.begin(), buffOut.begin() + (buffOut.size() - available_out));
			available_out = 0;
			available_in = 0;
			return {};
		}
	}

	return { "Unknown weird logic error. This should not happen. Something is fucked." };
}

Error Compress::brotliDecompressBuffer(const std::vector<uint8_t>& input, std::vector<uint8_t>& output) {

	if (!input.size()) return { "Empty input buffer" };
	if (output.size()) output.clear();

	BrotliDecompressStream br;
	auto streamStatus = BROTLI_DECODER_RESULT_NEEDS_MORE_INPUT;

	std::array<uint8_t, br.chunk> buffOut;

	size_t slider_in = 0;
	bool eob = false;

	auto next_in = input.data() + slider_in;
	size_t available_in = br.chunk;

	auto next_out = buffOut.data();
	size_t available_out = buffOut.size();

	while (true) {

		auto currOutSize = buffOut.size() - available_out;

		switch (streamStatus) {

			case BROTLI_DECODER_RESULT_NEEDS_MORE_INPUT: {

				if (eob) return { "Incomplete brotli stream" };

				next_in = input.data() + slider_in;
				eob = (slider_in + br.chunk) >= input.size();
				available_in = eob ? input.size() - slider_in : br.chunk;
				slider_in += available_in;

			} break;

			case BROTLI_DECODER_RESULT_NEEDS_MORE_OUTPUT: {

				output.insert(output.end(), buffOut.begin(), buffOut.begin() + currOutSize);
				next_out = buffOut.data();
				available_out = buffOut.size();

			} break;

			case BROTLI_DECODER_RESULT_SUCCESS: {

				output.insert(output.end(), buffOut.begin(), buffOut.begin() + currOutSize);

				available_in = 0;
				available_out = 0;

				if (slider_in < input.size()) return { "Unused data after brotli stream" };
				return {};

			} break;

			default: {

				auto errcode = BrotliDecoderGetErrorCode(br.stream);
				return { BrotliDecoderErrorString(errcode), errcode };

			} break;
		}

		streamStatus = BrotliDecoderDecompressStream(br.stream, &available_in, &next_in, &available_out, &next_out, 0);
	}

	return { "Unknown weird logic error. This should not happen. Something is fucked." };
}
