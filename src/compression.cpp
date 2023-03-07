/*
	zlib and brotli C++ wrappers for simple buffer compression
	2023 maddsua
	https://github.com/maddsua/
*/


#include <memory>
#include <array>

#include <brotli/encode.h>
#include <brotli/decode.h>

#include "../include/lambda/compression.hpp"


#define LAMBDA_BROTLI_CHUNK			(131072)	//	128k
#define LAMBDA_BROTLI_EXPCT_RATIO	(3)


/*
	ZLIB decompression stream class
*/

lambda::zlibDecompressStream::zlibDecompressStream() {
	datastream = new z_stream;
	memset(datastream, 0, sizeof(z_stream));
	tempBuffer = new uint8_t[chunkSize];
}

lambda::zlibDecompressStream::~zlibDecompressStream() {
	if (initialized) inflateEnd(datastream);
	delete datastream;
	delete tempBuffer;
}

bool lambda::zlibDecompressStream::init(int windowBits) {

	streamStatus = inflateInit2(datastream, windowBits);
	if (streamStatus != Z_OK) return false;

	initialized = true;
	return true;
}

bool lambda::zlibDecompressStream::done() {
	return (streamStatus == Z_STREAM_END);
}

bool lambda::zlibDecompressStream::error() {
	//	negative values are all errors
	return (streamStatus < Z_OK || streamStatus == Z_NEED_DICT);
}

bool lambda::zlibDecompressStream::doInflate(uint8_t* bufferIn, size_t dataInSize, std::vector <uint8_t>* bufferOut) {

	datastream->next_in = bufferIn;
	datastream->avail_in = dataInSize;

	do {
		datastream->avail_out = chunkSize;
		datastream->next_out = tempBuffer;
		
		streamStatus = inflate(datastream, Z_NO_FLUSH);
		if (error()) return false;

		bufferOut->insert(bufferOut->end(), tempBuffer, tempBuffer + (chunkSize - datastream->avail_out));
		
	} while (datastream->avail_out == 0);
	
	return streamStatus;
}


/*
	ZLIB compression stream class
*/

lambda::zlibCompressStream::zlibCompressStream() {
	datastream = new z_stream;
	memset(datastream, 0, sizeof(z_stream));
	tempBuffer = new uint8_t[chunkSize];
}

lambda::zlibCompressStream::~zlibCompressStream() {
	if (initialized) inflateEnd(datastream);
	delete datastream;
	delete tempBuffer;
}

bool lambda::zlibCompressStream::init(int compressLvl, int addHeader) {

	if (compressLvl < 0 || compressLvl > 9) compressLvl = Z_DEFAULT_COMPRESSION;

	streamStatus = deflateInit2(datastream, compressLvl, Z_DEFLATED, addHeader, def_memlvl, Z_DEFAULT_STRATEGY);
	if (streamStatus != Z_OK) return false;

	initialized = true;
	return true;
}

bool lambda::zlibCompressStream::done() {
	return (streamStatus == Z_STREAM_END);
}
bool lambda::zlibCompressStream::error() {
	//puts(std::to_string(streamStatus).c_str());
	//	negative values are errors
	return (streamStatus < Z_OK || streamStatus == Z_NEED_DICT);
}

bool lambda::zlibCompressStream::doDeflate(uint8_t* bufferIn, size_t dataInSize, std::vector <uint8_t>* bufferOut, bool finish) {

	datastream->next_in = bufferIn;
	datastream->avail_in = dataInSize;

	do {
		datastream->avail_out = chunkSize;
		datastream->next_out = tempBuffer;
		
		streamStatus = deflate(datastream, finish ? Z_FINISH : Z_NO_FLUSH);
		if (error()) return false;

		bufferOut->insert(bufferOut->end(), tempBuffer, tempBuffer + (chunkSize - datastream->avail_out));
		
	} while (datastream->avail_out == 0);
	
	return error();
}


/*
	zlib "wrapper" for de/compressing buffers
*/

/**
 * Compress std::string buffer as gzip. Successful if output size > 0
 * @param plain pointer to a string with original data
 * @param compressed pointer to a destination string, where the compressed data will be saved
*/
std::string lambda::gzCompress(const std::string* data, bool gzipHeader) {

	if (!data->size()) return {};

	zlibCompressStream stream;

	stream.init(Z_DEFAULT_COMPRESSION, gzipHeader ? zlibCompressStream::header_gz : zlibCompressStream::header_deflate);
	if (stream.error()) return {};

	std::vector <uint8_t> result;
		result.reserve(data->size() / zlibCompressStream::expect_ratio);

	bool streamEnd = false;
	size_t dataProcessed = 0;
	size_t partSize = 0;

	do {
		streamEnd = (dataProcessed + zlibCompressStream::chunkSize) >= data->size();
		partSize = streamEnd ? (data->size() - dataProcessed) : zlibCompressStream::chunkSize;

		stream.doDeflate((uint8_t*)(data->data() + dataProcessed), partSize, &result, streamEnd);
		dataProcessed += partSize;

	} while (!streamEnd && !stream.error());

	//	return empty string on error
	if (!stream.done() || stream.error()) return {};

	return std::string(result.begin(), result.end());
}

/**
 * Decompresses gzip-encoded std::string buffer. Successful if output size > 0
 * @param compressed pointer to a string with compressed data
 * @param plain pointer to a destination string, original data will get here
*/
std::string lambda::gzDecompress(const std::string* data) {

	if (!data->size()) return {};

	zlibDecompressStream stream;

	stream.init(zlibDecompressStream::winbit_auto);
	if (stream.error()) return {};

	std::vector <uint8_t> result;
		result.reserve(data->size() * zlibCompressStream::expect_ratio);

	bool streamEnd = false;
	size_t dataProcessed = 0;
	size_t partSize = 0;

	do {
		streamEnd = (dataProcessed + zlibCompressStream::chunkSize) >= data->size();
		partSize = streamEnd ? (data->size() - dataProcessed) : zlibCompressStream::chunkSize;

		stream.doInflate((uint8_t*)(data->data() + dataProcessed), partSize, &result);
		dataProcessed += partSize;

	} while (!streamEnd && !stream.error());

	//	return empty string on error
	if (!stream.done() || stream.error()) return {};

	return std::string(result.begin(), result.end());
}



/*	brotli strean classes	*/

lambda::brotliCompressStream::brotliCompressStream() {
	instance = BrotliEncoderCreateInstance(nullptr, nullptr, nullptr);
}
lambda::brotliCompressStream::~brotliCompressStream() {
	BrotliEncoderDestroyInstance((BrotliEncoderState*)instance);
}

bool lambda::brotliCompressStream::setQuality(int quality) {
	if (quality < BROTLI_MIN_QUALITY || quality > BROTLI_MAX_QUALITY) return false;
	return BrotliEncoderSetParameter((BrotliEncoderState*)instance, BROTLI_PARAM_QUALITY, quality);
}
bool lambda::brotliCompressStream::done() {
	return BrotliEncoderIsFinished((BrotliEncoderState*)instance);
}

bool lambda::brotliCompressStream::compressChunk(const uint8_t* chunk, const size_t chunkSize, std::vector <uint8_t>* bufferOut, bool finish) {

	size_t avail_in = chunkSize;
	auto operation = finish ? BROTLI_OPERATION_FINISH : BROTLI_OPERATION_PROCESS;

	while (avail_in) {
		uint8_t *next_out = nullptr;
		size_t avail_out = 0;

		if (!BrotliEncoderCompressStream((BrotliEncoderState*)instance, operation, &avail_in, &chunk, &avail_out, &next_out, nullptr)) return false;

		if (BrotliEncoderHasMoreOutput((BrotliEncoderState*)instance)) {
			size_t size = 0;
			auto brTempOutBuff = BrotliEncoderTakeOutput((BrotliEncoderState*)instance, &size);
			if (!brTempOutBuff) return false;
			bufferOut->insert(bufferOut->end(), brTempOutBuff, brTempOutBuff + size);
		}
	}

	return true;
}

lambda::brotliDecompressStream::brotliDecompressStream() {
	instance = BrotliDecoderCreateInstance(nullptr, nullptr, nullptr);
}
lambda::brotliDecompressStream::~brotliDecompressStream() {
	BrotliDecoderDestroyInstance((BrotliDecoderState*)instance);
}

bool lambda::brotliDecompressStream::done() {
	return BrotliDecoderIsFinished((BrotliDecoderState*)instance);
}

bool lambda::brotliDecompressStream::decompressChunk(const uint8_t* chunk, const size_t chunkSize, std::vector <uint8_t>* bufferOut) {

	size_t avail_in = chunkSize;

	while (avail_in) {
		uint8_t *next_out = nullptr;
		size_t avail_out = 0;

		if (!BrotliDecoderDecompressStream((BrotliDecoderState*)instance, &avail_in, &chunk, &avail_out, &next_out, nullptr)) return false;

		if (BrotliDecoderHasMoreOutput((BrotliDecoderState*)instance)) {
			size_t size = 0;
			auto brTempOutBuff = BrotliDecoderTakeOutput((BrotliDecoderState*)instance, &size);
			if (!brTempOutBuff) return false;
			bufferOut->insert(bufferOut->end(), brTempOutBuff, brTempOutBuff + size);
		}
	}

	return true;
}

/*	brotli wrapper for buffer de/compression	*/

/**
 * Compress data from std::string using Brotli. The return value "true" indicatess success
*/
std::string lambda::brCompress(const std::string* data) {

	if (!data->size()) return {};

	std::vector <uint8_t> result;
		result.reserve(data->size() / brotliCompressStream::expect_ratio);

	brotliCompressStream stream;

	bool streamEnd = false;
	size_t dataProcessed = 0;
	size_t partSize = 0;

	while (dataProcessed < data->size()) {
		streamEnd = (dataProcessed + brotliCompressStream::chunkSize) >= data->size();
		partSize = streamEnd ? (data->size() - dataProcessed) : brotliCompressStream::chunkSize;
		if (!stream.compressChunk((const uint8_t*)(data->data() + dataProcessed), partSize, &result, streamEnd)) return {};
		dataProcessed += partSize;
	}

	if (!stream.done()) {
		puts("stream not done!!!");
	}
	
	return std::string(result.begin(), result.end());;
}

/**
 * Decompress data from std::string using Brotli. The return value "true" indicatess success
*/
std::string lambda::brDecompress(const std::string* data) {

	std::vector <uint8_t> result;
		result.reserve(data->size() * brotliCompressStream::expect_ratio);

	brotliDecompressStream stream;

	bool streamEnd = false;
	size_t dataProcessed = 0;
	size_t partSize = 0;

	while (dataProcessed < data->size()) {
		streamEnd = (dataProcessed + brotliCompressStream::chunkSize) >= data->size();
		partSize = streamEnd ? (data->size() - dataProcessed) : brotliCompressStream::chunkSize;
		if (!stream.decompressChunk((const uint8_t*)(data->data() + dataProcessed), partSize, &result)) return {};
		dataProcessed += partSize;
	}

	if (!stream.done()) {
		puts("stream not done!!!");
	}
	
	return std::string(result.begin(), result.end());;
}
