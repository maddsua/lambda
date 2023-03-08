#include <zlib.h>
#include <brotli/encode.h>
#include <brotli/decode.h>

#include "../include/maddsua/compress.hpp"

/*
	zlib "wrapper" for de/compressing binary data
*/

/**
 * Compress data inside std::string using gzip. The return value "true" indicatess success
 * @param plain pointer to a string with original data
 * @param compressed pointer to a destination string, where the compressed data will be saved
*/
bool maddsua::gzCompress(const std::string* plain, std::string* compressed, bool gzipHeader) {

	if (!plain->size()) return false;

	z_stream zlibStream;
		zlibStream.zalloc = Z_NULL;
		zlibStream.zfree = Z_NULL;
		zlibStream.opaque = Z_NULL;

	auto zlibResult = deflateInit2(&zlibStream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, (gzipHeader ? MCOMP_Z_HEADER_GZ : MCOMP_Z_HEADER_Z), MCOMP_Z_MEMORY, Z_DEFAULT_STRATEGY);
		if (zlibResult != Z_OK) return false;

	compressed->resize(0);
	compressed->reserve(plain->size() / MCOMP_Z_EXPECT_RATIO);

	int zlibFlush;
	bool opresult = true;
	size_t carrierShift = 0;
	auto chunkIn = new uint8_t [MCOMP_Z_CHUNK];
	auto chunkOut = new uint8_t [MCOMP_Z_CHUNK];

	do {
		size_t partSize = MCOMP_Z_CHUNK;
		if ((MCOMP_Z_CHUNK + carrierShift) > plain->size()) partSize = (plain->size() - carrierShift);

		std::copy(plain->begin() + carrierShift, plain->begin() + carrierShift + partSize, chunkIn);

		carrierShift += MCOMP_Z_CHUNK;
		zlibFlush = (partSize < MCOMP_Z_CHUNK) ? Z_FINISH : Z_NO_FLUSH;
		zlibStream.avail_in = partSize;
		zlibStream.next_in = chunkIn;

		do {
			zlibStream.avail_out = MCOMP_Z_CHUNK;
			zlibStream.next_out = chunkOut;
			zlibResult = deflate(&zlibStream, zlibFlush);

			if (zlibResult == Z_STREAM_ERROR) {
				opresult = false;
				break;
			}

			compressed->insert(compressed->end(), chunkOut, chunkOut + (MCOMP_Z_CHUNK - zlibStream.avail_out));

		} while (zlibStream.avail_out == 0 && opresult);

		if (zlibStream.avail_in != 0) {
			opresult = false;
			break;
		}

	} while (zlibFlush != Z_FINISH && opresult);

	(void)deflateEnd(&zlibStream);
	delete chunkIn, chunkOut;

	return (zlibResult == Z_STREAM_END && opresult);
}

/**
 * Decompresses gzip-encoded data from std::string. The return value "true" indicatess success
 * @param compressed pointer to a string with compressed data
 * @param plain pointer to a destination string, original data will get here
*/
bool maddsua::gzDecompress(const std::string* compressed, std::string* plain) {

	if (!compressed->size()) return false;

	z_stream zlibStream;
		zlibStream.zalloc = Z_NULL;
		zlibStream.zfree = Z_NULL;
		zlibStream.opaque = Z_NULL;
		zlibStream.avail_in = 0;
		zlibStream.next_in = Z_NULL;

	auto zlibResult = inflateInit2(&zlibStream, MCOMP_Z_DECOM_AUTO);
		if (zlibResult != Z_OK) return false;

	plain->resize(0);
	plain->reserve(compressed->size() * MCOMP_Z_EXPECT_RATIO);

	size_t carrierShift = 0;
	bool opresult = true;
	auto chunkIn = new uint8_t [MCOMP_Z_CHUNK];
	auto chunkOut = new uint8_t [MCOMP_Z_CHUNK];

	do {
		size_t partSize = MCOMP_Z_CHUNK;
		if ((MCOMP_Z_CHUNK + carrierShift) > compressed->size()) partSize = (compressed->size() - carrierShift);

		std::copy(compressed->begin() + carrierShift, compressed->begin() + carrierShift + partSize, chunkIn);

		carrierShift += MCOMP_Z_CHUNK;
		zlibStream.avail_in = partSize;

		zlibStream.next_in = chunkIn;

		do {
			zlibStream.avail_out = MCOMP_Z_CHUNK;
			zlibStream.next_out = chunkOut;
			zlibResult = inflate(&zlibStream, Z_NO_FLUSH);

			//	negative values are errors
			if (zlibResult < Z_OK) {
				opresult = false;
				break;
			}

			plain->insert(plain->end(), chunkOut, chunkOut + (MCOMP_Z_CHUNK - zlibStream.avail_out));

		} while (zlibStream.avail_out == 0 && opresult);

	} while (zlibResult != Z_STREAM_END && opresult);

	(void)inflateEnd(&zlibStream);
	delete chunkIn, chunkOut;

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
bool maddsua::brDecompress(const std::string* compressed, std::string* plain) {

	if (!compressed->size()) return false;

	auto instance = BrotliDecoderCreateInstance(nullptr, nullptr, nullptr);
	
	BrotliDecoderResult opresult;
	auto chunkInSize = compressed->size();
	const uint8_t* bufferIn = reinterpret_cast<const uint8_t*>(compressed->data());
	auto bufferOut = new uint8_t [MCOMP_BR_CHUNK];

	plain->resize(0);
	plain->reserve(plain->size() * MCOMP_BR_EXPECT_RATIO);

	do {
		uint8_t* chunkOut = bufferOut;
		size_t chunkOutSize = MCOMP_BR_CHUNK;

		opresult = BrotliDecoderDecompressStream(instance, &chunkInSize, &bufferIn, &chunkOutSize, &chunkOut, nullptr);
		if (opresult == BROTLI_DECODER_RESULT_ERROR) break;
			
		plain->insert(plain->end(), bufferOut, bufferOut + (MCOMP_BR_CHUNK - chunkOutSize));

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
bool maddsua::brCompress(const std::string* plain, std::string* compressed) {

	if (!plain->size()) return false;

	auto instance = BrotliEncoderCreateInstance(nullptr, nullptr, nullptr);

	bool opresult;
	auto chunkInSize = plain->size();
	const uint8_t* bufferIn = reinterpret_cast<const uint8_t*>(plain->data());
	auto bufferOut = new uint8_t [MCOMP_BR_CHUNK];

	compressed->resize(0);
	compressed->reserve(plain->size() / MCOMP_BR_EXPECT_RATIO);

	do {
		uint8_t* chunkOut = bufferOut;
		size_t chunkOutSize = MCOMP_BR_CHUNK;

		opresult = BrotliEncoderCompressStream(instance, BROTLI_OPERATION_FINISH, &chunkInSize, &bufferIn, &chunkOutSize, &chunkOut, nullptr);
		if (!opresult) break;

		compressed->insert(compressed->end(), bufferOut, bufferOut + (MCOMP_BR_CHUNK - chunkOutSize));

	} while (chunkInSize != 0 || !BrotliEncoderIsFinished(instance));

	BrotliEncoderDestroyInstance(instance);
	delete bufferOut;

	if (!opresult) return false;

	return true;
}
