#include "../include/maddsua/bufferCompress.hpp"

/*
	zlib "wrapper" for de/compressing binary data
*/
bool maddsuaCompress::compressVector(const std::vector <uint8_t>* source, std::vector <uint8_t>* result, bool gzipHeader) {
	z_stream zlibStream;
		zlibStream.zalloc = Z_NULL;
		zlibStream.zfree = Z_NULL;
		zlibStream.opaque = Z_NULL;

	auto zlibResult = deflateInit2(&zlibStream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, gzipHeader ? ZLIB_MEXP_HEADER_GZ : ZLIB_MEXP_HEADER_Z, ZLIB_MEXP_MEM_MAX, Z_DEFAULT_STRATEGY);
		if (zlibResult != Z_OK) return false;

	result->reserve(source->size()/3);

	int zlibFlush;
	size_t carrierShift = 0;
	uint8_t chunkIn[ZLIB_MEXP_CHUNK];
	uint8_t chunkOut[ZLIB_MEXP_CHUNK];

	do {
		size_t partSize = ZLIB_MEXP_CHUNK;
			if((ZLIB_MEXP_CHUNK + carrierShift) > source->size()) partSize = (source->size() - carrierShift);

		std::copy(source->begin() + carrierShift, source->begin() + carrierShift + partSize, chunkIn);

		carrierShift += ZLIB_MEXP_CHUNK;
		zlibFlush = (partSize < ZLIB_MEXP_CHUNK) ? Z_FINISH : Z_NO_FLUSH;
		zlibStream.avail_in = partSize;
		zlibStream.next_in = chunkIn;

		do {
			zlibStream.avail_out = ZLIB_MEXP_CHUNK;
			zlibStream.next_out = chunkOut;
			zlibResult = deflate(&zlibStream, zlibFlush);
				if(zlibResult == Z_STREAM_ERROR) return false;

			result->insert(result->end(), chunkOut, chunkOut + (ZLIB_MEXP_CHUNK - zlibStream.avail_out));

		} while (zlibStream.avail_out == 0);
			if(zlibStream.avail_in != 0) return false;

	} while (zlibFlush != Z_FINISH);
		if(zlibResult != Z_STREAM_END) return false;

	return deflateEnd(&zlibStream) == Z_OK;
}

bool maddsuaCompress::decompressVector(const std::vector <uint8_t>* source, std::vector <uint8_t>* result) {

	z_stream zlibStream;
		zlibStream.zalloc = Z_NULL;
		zlibStream.zfree = Z_NULL;
		zlibStream.opaque = Z_NULL;
		zlibStream.avail_in = 0;
		zlibStream.next_in = Z_NULL;

	auto zlibResult = inflateInit2(&zlibStream, ZLIB_MEXP_DECOM_AUTO);
		if (zlibResult != Z_OK) return zlibResult;

	result->reserve(source->size()*3);

	size_t carrierShift = 0;
	uint8_t chunkIn[ZLIB_MEXP_CHUNK];
	uint8_t chunkOut[ZLIB_MEXP_CHUNK];

	do {
		size_t partSize = ZLIB_MEXP_CHUNK;
		if((ZLIB_MEXP_CHUNK + carrierShift) > source->size()) partSize = (source->size() - carrierShift);

		std::copy(source->begin() + carrierShift, source->begin() + carrierShift + partSize, chunkIn);

		carrierShift += ZLIB_MEXP_CHUNK;
		zlibStream.avail_in = partSize;

		zlibStream.next_in = chunkIn;

		do {
			zlibStream.avail_out = ZLIB_MEXP_CHUNK;
			zlibStream.next_out = chunkOut;
			zlibResult = inflate(&zlibStream, Z_NO_FLUSH);
				if(zlibResult == Z_STREAM_ERROR || zlibResult == Z_NEED_DICT || zlibResult == Z_MEM_ERROR|| zlibResult == Z_DATA_ERROR) return zlibResult;

			result->insert(result->end(), chunkOut, chunkOut + (ZLIB_MEXP_CHUNK - zlibStream.avail_out));

		} while (zlibStream.avail_out == 0);

	} while (zlibResult != Z_STREAM_END);

	(void)inflateEnd(&zlibStream);
	return zlibResult == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}