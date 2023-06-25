#include "compression.hpp"
#include <brotli/encode.h>
#include <brotli/decode.h>

Compress::BrotliStream::BrotliStream() {
	//	nothing to do this time
}

Compress::BrotliStream::~BrotliStream() {
	if (compressStream != nullptr)
		delete ((BrotliEncoderState*)compressStream);

	if (decompressStream != nullptr)
		delete ((BrotliDecoderState*)decompressStream);
}

bool Compress::BrotliStream::startCompression(int quality) {
	compressStream = BrotliEncoderCreateInstance(nullptr, nullptr, nullptr);
	if (quality < BROTLI_MIN_QUALITY || quality > BROTLI_MAX_QUALITY) quality = 5;
	compressStatus = BrotliEncoderSetParameter((BrotliEncoderState*)compressStream, BROTLI_PARAM_QUALITY, quality);
}
bool Compress::BrotliStream::startCompression() {
	return startCompression(5);
}

bool Compress::BrotliStream::compressionDone() {
	return BrotliEncoderIsFinished((BrotliEncoderState*)compressStream);
}

bool Compress::BrotliStream::compressChunk(uint8_t* bufferIn, size_t dataInSize, std::vector <uint8_t>* bufferOut, bool finish) {

	size_t avail_in = dataInSize;
	auto operation = finish ? BROTLI_OPERATION_FINISH : BROTLI_OPERATION_PROCESS;

	auto instance = (BrotliEncoderState*)compressStream;

	while (avail_in) {
		uint8_t* next_out = nullptr;
		size_t avail_out = 0;

		compressStatus = BrotliEncoderCompressStream(instance, operation, &avail_in, &bufferIn, &avail_out, &next_out, nullptr);
		if (!compressStatus) return false;

		if (BrotliEncoderHasMoreOutput(instance)) {

			size_t size = 0;

			auto temp = BrotliEncoderTakeOutput(instance, &size);
			if (!temp) return false;

			bufferOut->insert(bufferOut->end(), temp, temp + size);
		}
	}

	return true;
}

bool Compress::BrotliStream::compressBuffer(std::vector<uint8_t>* bufferIn, std::vector<uint8_t>* bufferOut) {

	if (!bufferIn->size()) return false;

	bufferOut->resize(0);
	bufferOut->reserve(bufferIn->size() / expect_ratio);

	bool streamEnd = false;
	size_t processed = 0;
	size_t chunkDataSize = 0;

	while (processed < bufferIn->size()) {
		streamEnd = (processed + chunkSize) >= bufferIn->size();
		chunkDataSize = streamEnd ? (bufferIn->size() - processed) : chunkSize;
		if (!compressChunk(bufferIn->data() + processed, chunkDataSize, bufferOut, streamEnd)) return false;
		processed += chunkDataSize;
	}

	return compressionDone();
}