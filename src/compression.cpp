/*
	maddsua's lambda
	A C++ HTTP server framework
	2023 https://github.com/maddsua/lambda
*/


#include <memory>

#include <zlib.h>
#include <brotli/encode.h>
#include <brotli/decode.h>

#include "../include/maddsua/compression.hpp"

#define LAMBDA_ZLIB_HEADER_Z		(8)
#define LAMBDA_ZLIB_HEADER_GZ		(26)
#define LAMBDA_ZLIB_DECOMP_GZ		(16)
#define LAMBDA_ZLIB_DECOM_AUTO		(32)
#define LAMBDA_ZLIB_RAW				(-15)
#define LAMBDA_ZLIB_MEMORY			(9)
#define LAMBDA_ZLIB_EXPECT_RATIO	(3)
#define LAMBDA_ZLIB_CHUNK			(131072)	//	128k

#define LAMBDA_BROTLI_CHUNK			(131072)	//	128k
#define LAMBDA_BROTLI_EXPCT_RATIO	(3)

/*
	zlib "wrapper" for de/compressing buffers
*/

/**
 * Compress data inside std::string using gzip. The return value "true" indicatess success
 * @param plain pointer to a string with original data
 * @param compressed pointer to a destination string, where the compressed data will be saved
*/
std::string lambda::compression::gzCompress(const std::string* plain, bool gzipHeader) {

	if (!plain->size()) return {};

	z_stream zlibStream;
		zlibStream.zalloc = Z_NULL;
		zlibStream.zfree = Z_NULL;
		zlibStream.opaque = Z_NULL;

	auto zlibResult = deflateInit2(&zlibStream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, (gzipHeader ? LAMBDA_ZLIB_HEADER_GZ : LAMBDA_ZLIB_HEADER_Z), LAMBDA_ZLIB_MEMORY, Z_DEFAULT_STRATEGY);
		if (zlibResult != Z_OK) return {};

	std::string result;
		result.reserve(plain->size() / LAMBDA_ZLIB_EXPECT_RATIO);

	int zlibFlush;
	bool opresult = true;
	size_t carrierShift = 0;

	auto chunkIn = new uint8_t [LAMBDA_ZLIB_CHUNK];
	auto chunkOut = new uint8_t [LAMBDA_ZLIB_CHUNK];

	do {
		size_t partSize = LAMBDA_ZLIB_CHUNK;
		if ((LAMBDA_ZLIB_CHUNK + carrierShift) > plain->size()) partSize = (plain->size() - carrierShift);

		std::copy(plain->begin() + carrierShift, plain->begin() + carrierShift + partSize, chunkIn);

		carrierShift += LAMBDA_ZLIB_CHUNK;
		zlibFlush = (partSize < LAMBDA_ZLIB_CHUNK) ? Z_FINISH : Z_NO_FLUSH;
		zlibStream.avail_in = partSize;
		zlibStream.next_in = chunkIn;

		do {
			zlibStream.avail_out = LAMBDA_ZLIB_CHUNK;
			zlibStream.next_out = chunkOut;
			zlibResult = deflate(&zlibStream, zlibFlush);

			if (zlibResult == Z_STREAM_ERROR) {
				opresult = false;
				break;
			}

			result.insert(result.end(), chunkOut, chunkOut + (LAMBDA_ZLIB_CHUNK - zlibStream.avail_out));

		} while (zlibStream.avail_out == 0 && opresult);

		if (zlibStream.avail_in != 0) {
			opresult = false;
			break;
		}

	} while (zlibFlush != Z_FINISH && opresult);

	(void)deflateEnd(&zlibStream);
	delete chunkIn;
	delete chunkOut;

	//	return empty string on error
	if (zlibResult != Z_STREAM_END || !opresult) return {};

	return result;
}

/**
 * Decompresses gzip-encoded data from std::string. The return value "true" indicatess success
 * @param compressed pointer to a string with compressed data
 * @param plain pointer to a destination string, original data will get here
*/
std::string lambda::compression::gzDecompress(const std::string* compressed) {

	if (!compressed->size()) return {};

	z_stream zlibStream;
		zlibStream.zalloc = Z_NULL;
		zlibStream.zfree = Z_NULL;
		zlibStream.opaque = Z_NULL;
		zlibStream.avail_in = 0;
		zlibStream.next_in = Z_NULL;

	auto zlibResult = inflateInit2(&zlibStream, LAMBDA_ZLIB_DECOM_AUTO);
		if (zlibResult != Z_OK) return {};

	std::string result;
		result.reserve(compressed->size() * LAMBDA_ZLIB_EXPECT_RATIO);

	size_t carrierShift = 0;
	bool opresult = true;
	auto chunkIn = new uint8_t [LAMBDA_ZLIB_CHUNK];
	auto chunkOut = new uint8_t [LAMBDA_ZLIB_CHUNK];

	do {
		size_t partSize = LAMBDA_ZLIB_CHUNK;
		if ((LAMBDA_ZLIB_CHUNK + carrierShift) > compressed->size()) partSize = (compressed->size() - carrierShift);

		std::copy(compressed->begin() + carrierShift, compressed->begin() + carrierShift + partSize, chunkIn);

		carrierShift += LAMBDA_ZLIB_CHUNK;
		zlibStream.avail_in = partSize;

		zlibStream.next_in = chunkIn;

		do {
			zlibStream.avail_out = LAMBDA_ZLIB_CHUNK;
			zlibStream.next_out = chunkOut;
			zlibResult = inflate(&zlibStream, Z_NO_FLUSH);

			//	negative values are errors
			if (zlibResult < Z_OK) {
				opresult = false;
				break;
			}

			result.insert(result.end(), chunkOut, chunkOut + (LAMBDA_ZLIB_CHUNK - zlibStream.avail_out));

		} while (zlibStream.avail_out == 0 && opresult);

	} while (zlibResult != Z_STREAM_END && opresult);

	(void)inflateEnd(&zlibStream);
	delete chunkIn;
	delete chunkOut;

	if (zlibResult != Z_STREAM_END || !opresult) return {};

	return result;
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
/*std::string lambda::compression::brDecompress(const std::string* data) {

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
std::string  lambda::compression::brCompress(const std::string* data) {

	if (!data->size()) return {};

	std::string result;
		result.resize(BrotliEncoderMaxCompressedSize(data->size()));

	size_t encodedSize = result.size();

	if (!BrotliEncoderCompress(BROTLI_DEFAULT_QUALITY, BROTLI_DEFAULT_WINDOW, BROTLI_DEFAULT_MODE, data->size(), (const uint8_t*)data->data(), &encodedSize, (uint8_t*)result.data())) return {};

	result.resize(encodedSize);
	result.shrink_to_fit();

	return result;
}
