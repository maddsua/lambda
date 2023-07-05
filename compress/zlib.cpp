#include "./compress.hpp"
#include "./streams.hpp"
#include <zlib.h>

using namespace Lambda;
using namespace Lambda::Compress;

Lambda::Error Compress::zlibCompressBuffer(const std::vector<uint8_t>& input, std::vector<uint8_t>& output) {
	return zlibCompressBuffer(input, output, 6, ZLIB_HEADER_GZ);
}

Lambda::Error Compress::zlibCompressBuffer(const std::vector<uint8_t>& input, std::vector<uint8_t>& output, int quality, ZlibWinbits header) {

	if (!input.size()) return Lambda::Error("Empty input buffer");

	if (quality < Z_NO_COMPRESSION) quality = Z_NO_COMPRESSION;
	else if (quality > Z_BEST_COMPRESSION) quality = Z_BEST_COMPRESSION;

	output.clear();

	auto zlib = ZlibCompressStream(quality, header);

	size_t cursor_in = 0;
	size_t cursor_out = 0;
	bool eob = false;
	int opres = Z_OK;

	do {

		eob = cursor_in + zlib.chunk >= input.size();
		zlib.stream->avail_in = eob ? input.size() - cursor_in : zlib.chunk;
		zlib.stream->next_in = (uint8_t*)(input.data() + cursor_in);
		cursor_in += zlib.stream->avail_in;

		do {

			zlib.stream->avail_out = zlib.chunk;
			if ((output.size() - cursor_out) < zlib.stream->avail_out)
				output.resize(output.size() + zlib.stream->avail_out);
			zlib.stream->next_out = output.data() + cursor_out;

			opres = deflate(zlib.stream, eob ? Z_FINISH : Z_NO_FLUSH);
			if (opres < 0) return Lambda::Error("deflate stream error", opres);

			cursor_out += zlib.chunk - zlib.stream->avail_out;

		} while (zlib.stream->avail_out == 0);

	} while (!eob);

	if (opres != Z_STREAM_END) return Lambda::Error("deflate stream failed to properly finish", opres);

	output.resize(cursor_out);
	output.shrink_to_fit();
	
	return {};
}

Lambda::Error Compress::zlibDecompressBuffer(const std::vector<uint8_t>& input, std::vector<uint8_t>& output) {

	if (!input.size()) return Lambda::Error("Empty input buffer");
	output.clear();

	auto zlib = ZlibDecompressStream(ZLIB_OPEN_AUTO);

	size_t cursor_in = 0;
	size_t cursor_out = 0;
	bool eob = false;
	int opres = Z_OK;

	do {

		eob = cursor_in + zlib.chunk >= input.size();
		zlib.stream->avail_in = eob ? input.size() - cursor_in : zlib.chunk;
		zlib.stream->next_in = (uint8_t*)(input.data() + cursor_in);
		cursor_in += zlib.stream->avail_in;

		do {

			zlib.stream->avail_out = zlib.chunk;
			if ((output.size() - cursor_out) < zlib.stream->avail_out)
				output.resize(output.size() + zlib.stream->avail_out);
			zlib.stream->next_out = output.data() + cursor_out;

			opres = inflate(zlib.stream, Z_NO_FLUSH);
			if (opres < Z_OK || opres > Z_STREAM_END) return Lambda::Error("inflate stream error", opres);

			cursor_out += zlib.chunk - zlib.stream->avail_out;

		} while (zlib.stream->avail_out == 0);

	} while (!eob && opres != Z_STREAM_END);

	if (opres != Z_STREAM_END) return Lambda::Error("inflate stream failed to properly finish", opres);

	output.resize(cursor_out);
	output.shrink_to_fit();
	
	return {};
}
