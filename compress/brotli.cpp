#include "./compress.hpp"
#include <brotli/encode.h>
#include <brotli/decode.h>

Lambda::Compress::BrotliStream::BrotliStream() {
	//	nothing to do this time
}

Lambda::Compress::BrotliStream::~BrotliStream() {
	if (compressStream != nullptr) {
		BrotliEncoderDestroyInstance((BrotliEncoderState*)compressStream);
		//delete ((BrotliEncoderState*)compressStream);
	}

	if (decompressStream != nullptr) {
		BrotliDecoderDestroyInstance((BrotliDecoderState*)decompressStream);
		//delete ((BrotliDecoderState*)decompressStream);
	}
}

bool Lambda::Compress::BrotliStream::startCompression(int quality) {
	compressStream = BrotliEncoderCreateInstance(nullptr, nullptr, nullptr);
	if (quality < BROTLI_MIN_QUALITY || quality > BROTLI_MAX_QUALITY) quality = 5;
	compressStatus = BrotliEncoderSetParameter((BrotliEncoderState*)compressStream, BROTLI_PARAM_QUALITY, quality);
	return compressStatus;
}
bool Lambda::Compress::BrotliStream::startCompression() {
	return startCompression(5);
}

bool Lambda::Compress::BrotliStream::compressionDone() {
	return BrotliEncoderIsFinished((BrotliEncoderState*)compressStream);
}

bool Lambda::Compress::BrotliStream::compressionError() {
	return !compressStatus;
}

int Lambda::Compress::BrotliStream::compressionStatus() {
	return compressStatus;
}

bool Lambda::Compress::BrotliStream::compressChunk(const uint8_t* bufferIn, size_t dataInSize, std::vector <uint8_t>* bufferOut, bool finish) {

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

bool Lambda::Compress::BrotliStream::compressBuffer(const std::vector<uint8_t>* bufferIn, std::vector<uint8_t>* bufferOut) {

	if (!bufferIn->size()) return false;

	bufferOut->clear();
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

bool Lambda::Compress::BrotliStream::startDecompression() {
	decompressStream = BrotliDecoderCreateInstance(nullptr, nullptr, nullptr);
	decompressStatus = BROTLI_DECODER_RESULT_SUCCESS;
	return decompressStatus != BROTLI_DECODER_RESULT_ERROR;
}

bool Lambda::Compress::BrotliStream::decompressionDone() {
	return BrotliDecoderIsFinished((BrotliDecoderState*)decompressStream);
}

bool Lambda::Compress::BrotliStream::decompressionError() {
	return !decompressStatus;
}

int Lambda::Compress::BrotliStream::decompressionStatus() {
	return decompressStatus;
}

bool Lambda::Compress::BrotliStream::decompressChunk(const uint8_t* bufferIn, size_t dataInSize, std::vector <uint8_t>* bufferOut) {

	auto instance = (BrotliDecoderState*)decompressStream;

	size_t avail_in = dataInSize;

	while (avail_in) {
		uint8_t* next_out = nullptr;
		size_t avail_out = 0;

		if (!BrotliDecoderDecompressStream(instance, &avail_in, &bufferIn, &avail_out, &next_out, nullptr)) return false;

		if (BrotliDecoderHasMoreOutput(instance)) {

			size_t size = 0;

			auto temp = BrotliDecoderTakeOutput(instance, &size);
			if (!temp) return false;

			bufferOut->insert(bufferOut->end(), temp, temp + size);
		}
	}

	return true;
}

bool Lambda::Compress::BrotliStream::decompressBuffer(const std::vector<uint8_t>* bufferIn, std::vector<uint8_t>* bufferOut) {

	if (!bufferIn->size()) return false;

	bufferOut->clear();
	bufferOut->reserve(bufferIn->size() * expect_ratio);

	bool streamEnd = false;
	size_t processed = 0;
	size_t chunkDataSize = 0;

	while (processed < bufferIn->size()) {
		streamEnd = (processed + chunkSize) >= bufferIn->size();
		chunkDataSize = streamEnd ? (bufferIn->size() - processed) : chunkSize;
		if (!decompressChunk(bufferIn->data() + processed, chunkDataSize, bufferOut)) return false;
		processed += chunkDataSize;
	}

	return decompressionDone();
}
