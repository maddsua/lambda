#include <zlib.h>
#include <brotli/encode.h>
#include <brotli/decode.h>

#include "../include/maddsua/lambda.hpp"

#define LAMBDA_ZLIB_HEADER_Z		(8)
#define LAMBDA_ZLIB_HEADER_GZ		(26)
#define LAMBDA_ZLIB_DECOMP_GZ		(16)
#define LAMBDA_ZLIB_DECOM_AUTO		(32)
#define LAMBDA_ZLIB_RAW				(-15)
#define LAMBDA_ZLIB_MEMORY			(9)
#define LAMBDA_ZLIB_EXPECT_RATIO	(3)
#define LAMBDA_ZLIB_CHUNK			(131072)	//	128k

#define LAMBDA_BROTLICHUNK			(131072)	//	128k
#define LAMBDA_BROTLIEXPECT_RATIO	(3)

/*
	zlib "wrapper" for de/compressing binary data
*/

/**
 * Compress data inside std::string using gzip. The return value "true" indicatess success
 * @param plain pointer to a string with original data
 * @param compressed pointer to a destination string, where the compressed data will be saved
*/
bool lambda::compression::gzCompress(const std::string* plain, std::string* compressed, bool gzipHeader) {

	if (!plain->size()) return false;

	z_stream zlibStream;
		zlibStream.zalloc = Z_NULL;
		zlibStream.zfree = Z_NULL;
		zlibStream.opaque = Z_NULL;

	auto zlibResult = deflateInit2(&zlibStream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, (gzipHeader ? LAMBDA_ZLIB_HEADER_GZ : LAMBDA_ZLIB_HEADER_Z), LAMBDA_ZLIB_MEMORY, Z_DEFAULT_STRATEGY);
		if (zlibResult != Z_OK) return false;

	compressed->resize(0);
	compressed->reserve(plain->size() / LAMBDA_ZLIB_EXPECT_RATIO);

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

			compressed->insert(compressed->end(), chunkOut, chunkOut + (LAMBDA_ZLIB_CHUNK - zlibStream.avail_out));

		} while (zlibStream.avail_out == 0 && opresult);

		if (zlibStream.avail_in != 0) {
			opresult = false;
			break;
		}

	} while (zlibFlush != Z_FINISH && opresult);

	(void)deflateEnd(&zlibStream);
	delete chunkIn;
	delete chunkOut;

	return (zlibResult == Z_STREAM_END && opresult);
}

/**
 * Decompresses gzip-encoded data from std::string. The return value "true" indicatess success
 * @param compressed pointer to a string with compressed data
 * @param plain pointer to a destination string, original data will get here
*/
bool lambda::compression::gzDecompress(const std::string* compressed, std::string* plain) {

	if (!compressed->size()) return false;

	z_stream zlibStream;
		zlibStream.zalloc = Z_NULL;
		zlibStream.zfree = Z_NULL;
		zlibStream.opaque = Z_NULL;
		zlibStream.avail_in = 0;
		zlibStream.next_in = Z_NULL;

	auto zlibResult = inflateInit2(&zlibStream, LAMBDA_ZLIB_DECOM_AUTO);
		if (zlibResult != Z_OK) return false;

	plain->resize(0);
	plain->reserve(compressed->size() * LAMBDA_ZLIB_EXPECT_RATIO);

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

			plain->insert(plain->end(), chunkOut, chunkOut + (LAMBDA_ZLIB_CHUNK - zlibStream.avail_out));

		} while (zlibStream.avail_out == 0 && opresult);

	} while (zlibResult != Z_STREAM_END && opresult);

	(void)inflateEnd(&zlibStream);
	delete chunkIn;
	delete chunkOut;

	return (zlibResult == Z_STREAM_END && opresult);
}

/*
	brotli "wrapper" for de/compressing binary data
	Seriously, the procedure is fckd. I thought that zlib is weird, but not comparing to this.
	By the way, any clues why there is no function to determine required buffer size for one-shot decompression?
	 How am I supposed to know the size of reconstructed data? Should I guess it, or try to fit in pre-allocated 10k buffer?
	By the way 2, why not just dynamically fckng allocate the memory needed? Am I the only one person to compress
	 in-memory stuff (http requests in this case)?
	 
	No, really. Both brotli and zlib have functions that work with complete buffers of known size. But they both fail to make
	 them usable.
*/

/**
 * Decompresses brotli-encoded data from std::string. The return value "true" indicatess success
 * @param compressed pointer to a string with compressed data
 * @param plain pointer to a destination string, original data will get here
*/
bool lambda::compression::brDecompress(const std::string* compressed, std::string* plain) {

	if (!compressed->size()) return false;

	auto instance = BrotliDecoderCreateInstance(nullptr, nullptr, nullptr);
	
	BrotliDecoderResult opresult;
	auto chunkInSize = compressed->size();
	const uint8_t* bufferIn = reinterpret_cast<const uint8_t*>(compressed->data());
	auto bufferOut = new uint8_t [LAMBDA_BROTLICHUNK];

	plain->resize(0);
	plain->reserve(plain->size() * LAMBDA_BROTLIEXPECT_RATIO);

	do {
		uint8_t* chunkOut = bufferOut;
		size_t chunkOutSize = LAMBDA_BROTLICHUNK;

		opresult = BrotliDecoderDecompressStream(instance, &chunkInSize, &bufferIn, &chunkOutSize, &chunkOut, nullptr);
		if (opresult == BROTLI_DECODER_RESULT_ERROR) break;
			
		plain->insert(plain->end(), bufferOut, bufferOut + (LAMBDA_BROTLICHUNK - chunkOutSize));

	} while (chunkInSize != 0 || opresult != BROTLI_DECODER_RESULT_SUCCESS);

	BrotliDecoderDestroyInstance(instance);
	delete bufferOut;

	if (opresult != BROTLI_DECODER_RESULT_SUCCESS) return false;

	return true;
}

/**
 * Compress data inside std::string using brotli. The return value "true" indicatess success
 * @param plain pointer to a string with original data
 * @param compressed pointer to a destination string, where the compressed data will be saved
*/
bool lambda::compression::brCompress(const std::string* plain, std::string* compressed) {

	if (!plain->size()) return false;

	auto instance = BrotliEncoderCreateInstance(nullptr, nullptr, nullptr);

	bool opresult;
	auto chunkInSize = plain->size();
	const uint8_t* bufferIn = reinterpret_cast<const uint8_t*>(plain->data());
	auto bufferOut = new uint8_t [LAMBDA_BROTLICHUNK];

	compressed->resize(0);
	compressed->reserve(plain->size() / LAMBDA_BROTLIEXPECT_RATIO);

	do {
		uint8_t* chunkOut = bufferOut;
		size_t chunkOutSize = LAMBDA_BROTLICHUNK;

		opresult = BrotliEncoderCompressStream(instance, BROTLI_OPERATION_FINISH, &chunkInSize, &bufferIn, &chunkOutSize, &chunkOut, nullptr);
		if (!opresult) break;

		compressed->insert(compressed->end(), bufferOut, bufferOut + (LAMBDA_BROTLICHUNK - chunkOutSize));

	} while (chunkInSize != 0 || !BrotliEncoderIsFinished(instance));

	BrotliEncoderDestroyInstance(instance);
	delete bufferOut;

	if (!opresult) return false;

	return true;
}
