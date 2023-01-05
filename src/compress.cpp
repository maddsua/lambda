#include "../include/maddsua/compress.hpp"

/*
	zlib "wrapper" for de/compressing binary data
*/
bool maddsua::gzCompress(const std::vector <uint8_t>* plain, std::vector <uint8_t>* compressed, bool gzipHeader) {
	z_stream zlibStream;
		zlibStream.zalloc = Z_NULL;
		zlibStream.zfree = Z_NULL;
		zlibStream.opaque = Z_NULL;

	auto writeHeader = (gzipHeader ? ZLIB_MEXP_HEADER_GZ : ZLIB_MEXP_HEADER_Z);
	auto zlibResult = deflateInit2(&zlibStream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, writeHeader, ZLIB_MEXP_MEMORY, Z_DEFAULT_STRATEGY);
		if (zlibResult != Z_OK) return false;

	compressed->resize(0);
	compressed->reserve(plain->size() / ZLIB_MEXP_EXPECT_RATIO);

	int zlibFlush;
	size_t carrierShift = 0;
	uint8_t chunkIn[ZLIB_MEXP_CHUNK];
	uint8_t chunkOut[ZLIB_MEXP_CHUNK];

	do {
		size_t partSize = ZLIB_MEXP_CHUNK;
			if((ZLIB_MEXP_CHUNK + carrierShift) > plain->size()) partSize = (plain->size() - carrierShift);

		std::copy(plain->begin() + carrierShift, plain->begin() + carrierShift + partSize, chunkIn);

		carrierShift += ZLIB_MEXP_CHUNK;
		zlibFlush = (partSize < ZLIB_MEXP_CHUNK) ? Z_FINISH : Z_NO_FLUSH;
		zlibStream.avail_in = partSize;
		zlibStream.next_in = chunkIn;

		do {
			zlibStream.avail_out = ZLIB_MEXP_CHUNK;
			zlibStream.next_out = chunkOut;
			zlibResult = deflate(&zlibStream, zlibFlush);
				if (zlibResult == Z_STREAM_ERROR) return false;

			compressed->insert(compressed->end(), chunkOut, chunkOut + (ZLIB_MEXP_CHUNK - zlibStream.avail_out));

		} while (zlibStream.avail_out == 0);

		if (zlibStream.avail_in != 0) return false;

	} while (zlibFlush != Z_FINISH);

	(void)deflateEnd(&zlibStream);

	return (zlibResult == Z_STREAM_END);
}

bool maddsua::gzDecompress(const std::vector <uint8_t>* compressed, std::vector <uint8_t>* plain) {

	z_stream zlibStream;
		zlibStream.zalloc = Z_NULL;
		zlibStream.zfree = Z_NULL;
		zlibStream.opaque = Z_NULL;
		zlibStream.avail_in = 0;
		zlibStream.next_in = Z_NULL;

	auto zlibResult = inflateInit2(&zlibStream, ZLIB_MEXP_DECOM_AUTO);
		if (zlibResult != Z_OK) return false;

	plain->resize(0);
	plain->reserve(compressed->size() * ZLIB_MEXP_EXPECT_RATIO);

	size_t carrierShift = 0;
	uint8_t chunkIn[ZLIB_MEXP_CHUNK];
	uint8_t chunkOut[ZLIB_MEXP_CHUNK];

	do {
		size_t partSize = ZLIB_MEXP_CHUNK;
		if((ZLIB_MEXP_CHUNK + carrierShift) > compressed->size()) partSize = (compressed->size() - carrierShift);

		std::copy(compressed->begin() + carrierShift, compressed->begin() + carrierShift + partSize, chunkIn);

		carrierShift += ZLIB_MEXP_CHUNK;
		zlibStream.avail_in = partSize;

		zlibStream.next_in = chunkIn;

		do {
			zlibStream.avail_out = ZLIB_MEXP_CHUNK;
			zlibStream.next_out = chunkOut;
			zlibResult = inflate(&zlibStream, Z_NO_FLUSH);

			//	negative values are errors
			if (zlibResult < Z_OK) return false;

			plain->insert(plain->end(), chunkOut, chunkOut + (ZLIB_MEXP_CHUNK - zlibStream.avail_out));

		} while (zlibStream.avail_out == 0);

	} while (zlibResult != Z_STREAM_END);

	(void)inflateEnd(&zlibStream);

	return (zlibResult == Z_STREAM_END);
}