#include "./compress.hpp"
#include "./streams.hpp"

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
	
	size_t cursor_in = 0;
	size_t cursor_out = 0;
	bool eob = false;

	auto next_in = input.data() + cursor_in;
	size_t available_in = 0;

	auto next_out = output.data() + cursor_out;
	size_t available_out = 0;

	while (true) {

		if (available_in == 0 && !eob) {
			eob = (cursor_in + br.chunk) >= input.size();
			next_in = input.data() + cursor_in;
			available_in = eob ? input.size() - cursor_in : br.chunk;
			cursor_in += available_in;
		}

		if (available_out == 0) {
			available_out = br.chunk;
			if (output.size() - cursor_out < available_out)
				output.resize(output.size() + available_out);
			next_out = output.data() + cursor_out;
			cursor_out += available_out;
		}

		if (BrotliEncoderIsFinished(br.stream)) {
			output.resize(cursor_out - available_out);
			output.shrink_to_fit();
			available_out = 0;
			available_in = 0;
			return {};
		}

		if (!BrotliEncoderCompressStream(br.stream, eob ? BROTLI_OPERATION_FINISH : BROTLI_OPERATION_PROCESS, &available_in, &next_in, &available_out, &next_out, nullptr)) {
			throw Lambda::Error("brotli encoder failed");
		}
	}

	return { "Unknown weird logic error. This should not happen. Something is fucked." };
}

Error Compress::brotliDecompressBuffer(const std::vector<uint8_t>& input, std::vector<uint8_t>& output) {

	if (!input.size()) return { "Empty input buffer" };
	if (output.size()) output.clear();

	BrotliDecompressStream br;
	auto streamStatus = BROTLI_DECODER_RESULT_NEEDS_MORE_INPUT;

	size_t cursor_in = 0;
	size_t cursor_out = 0;
	bool eob = false;

	auto next_in = input.data() + cursor_in;
	size_t available_in = br.chunk;

	auto next_out = output.data() + cursor_out;
	size_t available_out = 0;

	while (true) {

		switch (streamStatus) {

			case BROTLI_DECODER_RESULT_NEEDS_MORE_INPUT: {

				if (eob) return { "Incomplete brotli stream" };

				next_in = input.data() + cursor_in;
				eob = (cursor_in + br.chunk) >= input.size();
				available_in = eob ? input.size() - cursor_in : br.chunk;
				cursor_in += available_in;

			} break;

			case BROTLI_DECODER_RESULT_NEEDS_MORE_OUTPUT: {

				available_out = br.chunk;
				if (output.size() - cursor_out < available_out)
					output.resize(output.size() + available_out);
				next_out = output.data() + cursor_out;
				cursor_out += available_out;

			} break;

			case BROTLI_DECODER_RESULT_SUCCESS: {

				output.resize(cursor_out - available_out);
				output.shrink_to_fit();

				available_in = 0;
				available_out = 0;

				if (cursor_in < input.size()) return { "Unused data after brotli stream" };
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
