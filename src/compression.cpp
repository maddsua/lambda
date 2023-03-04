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
	size_t carrierShift = 0;
	size_t partSize = 0;
	std::array <uint8_t, zlibCompressStream::chunkSize> chunkIn;

	do {
		streamEnd = (carrierShift + zlibCompressStream::chunkSize) >= data->size();
		partSize = streamEnd ? (data->size() - carrierShift) : zlibCompressStream::chunkSize;

		memcpy(chunkIn.data(), data->data() + carrierShift, partSize);
		carrierShift += partSize;

		stream.doDeflate(chunkIn.data(), partSize, &result, streamEnd);

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
		result.reserve(data->size() / zlibCompressStream::expect_ratio);

	bool streamEnd = false;
	size_t carrierShift = 0;
	size_t partSize = 0;
	std::array <uint8_t, zlibCompressStream::chunkSize> chunkIn;

	do {
		streamEnd = (carrierShift + zlibCompressStream::chunkSize) >= data->size();
		partSize = streamEnd ? (data->size() - carrierShift) : zlibCompressStream::chunkSize;

		memcpy(chunkIn.data(), data->data() + carrierShift, partSize);
		carrierShift += partSize;

		stream.doInflate(chunkIn.data(), partSize, &result);

	} while (!streamEnd && !stream.error());

	//	return empty string on error
	if (!stream.done() || stream.error()) return {};

	return std::string(result.begin(), result.end());
}


/*
	brotli "wrapper" for de/compressing binary data
	Seriously, the procedure is fckd. I thought that zlib is weird, but not comparing to this.
	By the way, any clues why there is no function to determine required buffer size for one-shot decompression?
	 How am I supposed to know the size of reconstructed data? Should I guess it, or try to fit in pre-allocated 10k buffer?
	By the way 2, why not just dynamically fckng allocate the memory needed? Am I the only one person to compress
	 in-memory stuff (http requests in this case)?
	 
	No, really. Both brotli and zlib have functions that work with buffers of known size. But they both fail to make
	 these functions any usable.
*/

/**
 * Decompresses brotli-encoded data from std::string. The return value "true" indicatess success
*/
/*std::string maddsua::brDecompress(const std::string* data) {

	if (!data->size()) return {};

	auto instance = BrotliDecoderCreateInstance(nullptr, nullptr, nullptr);
	
	BrotliDecoderResult opresult;
	auto chunkInSize = data->size();
	const uint8_t* bufferIn = reinterpret_cast<const uint8_t*>(data->data());
	auto bufferOut = new uint8_t [LAMBDA_BROTLI_CHUNK];

	std::string result;
		result.reserve(data->size() * LAMBDA_BROTLI_EXPCT_RATIO);

	do {
		uint8_t* chunkOut = bufferOut;
		size_t chunkOutSize = LAMBDA_BROTLI_CHUNK;

		opresult = BrotliDecoderDecompressStream(instance, &chunkInSize, &bufferIn, &chunkOutSize, &chunkOut, nullptr);
		if (opresult == BROTLI_DECODER_RESULT_ERROR) break;
			
		result.insert(result.end(), bufferOut, bufferOut + (LAMBDA_BROTLI_CHUNK - chunkOutSize));

	} while (chunkInSize != 0 || opresult != BROTLI_DECODER_RESULT_SUCCESS);

	BrotliDecoderDestroyInstance(instance);
	delete bufferOut;

	if (opresult != BROTLI_DECODER_RESULT_SUCCESS) return {};

	return result;
}*/

/**
 * Compress data from std::string using Brotli. The return value "true" indicatess success
*/
std::string  lambda::brCompress(const std::string* data) {

	if (!data->size()) return {};

	std::string result;
		result.resize(BrotliEncoderMaxCompressedSize(data->size()));

	size_t encodedSize = result.size();

	if (!BrotliEncoderCompress(BROTLI_DEFAULT_QUALITY, BROTLI_DEFAULT_WINDOW, BROTLI_DEFAULT_MODE, data->size(), (const uint8_t*)data->data(), &encodedSize, (uint8_t*)result.data())) return {};

	result.resize(encodedSize);
	result.shrink_to_fit();

	return result;
}
