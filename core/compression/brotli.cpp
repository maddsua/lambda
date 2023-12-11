#include "../compression.hpp"
#include "./streams.hpp"

using namespace Lambda;

std::vector<uint8_t> Compress::brotliCompressBuffer(const std::vector<uint8_t>& input, Quality quality) {

	if (!input.size()) return {};

	try {

		auto qualityAsLevel = static_cast<std::underlying_type_t<Quality>>(quality);
		int compressionLevel = (((double)BROTLI_MAX_QUALITY / static_cast<std::underlying_type_t<Quality>>(Quality::Max)) * qualityAsLevel);

		if (compressionLevel < BROTLI_MIN_QUALITY || compressionLevel > BROTLI_MAX_QUALITY)
			throw std::runtime_error("unable to convert compression quality to level: (" + std::to_string(compressionLevel) + ") is out of bounds)");

		BrotliCompressStream br;

		if (!BrotliEncoderSetParameter(br.stream, BROTLI_PARAM_QUALITY, qualityAsLevel))
			throw std::runtime_error("compression quality unrecognized");

		std::vector<uint8_t> compressed;
		
		size_t cursor_in = 0;
		size_t cursor_out = 0;
		bool eob = false;

		auto next_in = input.data() + cursor_in;
		size_t available_in = 0;

		auto next_out = compressed.data() + cursor_out;
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
				if (compressed.size() - cursor_out < available_out)
					compressed.resize(compressed.size() + available_out);
				next_out = compressed.data() + cursor_out;
				cursor_out += available_out;
			}

			if (BrotliEncoderIsFinished(br.stream)) {
				compressed.resize(cursor_out - available_out);
				compressed.shrink_to_fit();
				available_out = 0;
				available_in = 0;
				return compressed;
			}

			auto brotliEncoderOp = eob ? BROTLI_OPERATION_FINISH : BROTLI_OPERATION_PROCESS;
			if (!BrotliEncoderCompressStream(br.stream, brotliEncoderOp, &available_in, &next_in, &available_out, &next_out, nullptr))
				throw std::runtime_error("unrecoverable encoder error");
		}

		throw std::runtime_error("logic error");

	} catch(const std::exception& e) {
		throw std::runtime_error(std::string("brotliCompressBuffer failed: ") + e.what());
	}
}

std::vector<uint8_t> Compress::brotliDecompressBuffer(const std::vector<uint8_t>& input) {

	if (!input.size()) return {};

	try {

		BrotliDecompressStream br;
		auto streamStatus = BROTLI_DECODER_RESULT_NEEDS_MORE_INPUT;
		std::vector<uint8_t> decompressed;

		size_t cursor_in = 0;
		size_t cursor_out = 0;
		bool eob = false;

		auto next_in = input.data() + cursor_in;
		size_t available_in = br.chunk;

		auto next_out = decompressed.data() + cursor_out;
		size_t available_out = 0;

		while (true) {

			switch (streamStatus) {

				case BROTLI_DECODER_RESULT_NEEDS_MORE_INPUT: {

					if (eob) std::runtime_error("Incomplete brotli stream");

					next_in = input.data() + cursor_in;
					eob = (cursor_in + br.chunk) >= input.size();
					available_in = eob ? input.size() - cursor_in : br.chunk;
					cursor_in += available_in;

				} break;

				case BROTLI_DECODER_RESULT_NEEDS_MORE_OUTPUT: {

					available_out = br.chunk;
					if (decompressed.size() - cursor_out < available_out)
						decompressed.resize(decompressed.size() + available_out);
					next_out = decompressed.data() + cursor_out;
					cursor_out += available_out;

				} break;

				case BROTLI_DECODER_RESULT_SUCCESS: {

					decompressed.resize(cursor_out - available_out);
					decompressed.shrink_to_fit();

					available_in = 0;
					available_out = 0;

					if (cursor_in < input.size()) std::runtime_error("Unused data after brotli stream");
					return decompressed;

				} break;

				default: {

					auto errcode = BrotliDecoderGetErrorCode(br.stream);
					throw std::runtime_error(BrotliDecoderErrorString(errcode));

				} break;
			}

			streamStatus = BrotliDecoderDecompressStream(br.stream, &available_in, &next_in, &available_out, &next_out, 0);
		}

		throw std::runtime_error("Unknown weird logic error. This should not happen. Something is fucked.");
		
	} catch(const std::exception& e) {
		throw std::runtime_error(std::string("brotliDecompressBuffer failed: ") + e.what());
	}
}
