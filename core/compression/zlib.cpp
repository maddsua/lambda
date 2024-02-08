#include "./compression.hpp"
#include "./streams.hpp"

#include <array>

using namespace Lambda;
using namespace Lambda::Compress;
using namespace Lambda::Compress::Streams;

ZlibStream::ZlibStream() {
	memset(&this->stream, 0, sizeof(z_stream));
}

ZlibCompressStream::ZlibCompressStream(int compression, int winbits) {
	auto initResult = deflateInit2(&this->stream, compression, Z_DEFLATED, winbits, 8, Z_DEFAULT_STRATEGY);
	if (initResult != Z_OK) throw std::runtime_error("Could not initialize deflate (zlib error code " + std::to_string(initResult) + ')');
}
ZlibCompressStream::~ZlibCompressStream() {
	(void)deflateEnd(&this->stream);
}

ZlibDecompressStream::ZlibDecompressStream(int winbits) {
	auto initResult = inflateInit2(&this->stream, winbits);
	if (initResult != Z_OK) throw std::runtime_error("Could not initialize inflate (zlib error code " + std::to_string(initResult) + ')');
}
ZlibDecompressStream::~ZlibDecompressStream() {
	(void)inflateEnd(&this->stream);
}

std::vector<uint8_t> Compress::zlibCompressBuffer(const std::vector<uint8_t>& input, Quality quality, ZlibSetHeader header) {

	if (!input.size()) return {};

	try {

		auto qualityAsLevel = static_cast<std::underlying_type_t<Quality>>(quality);
		if (qualityAsLevel < Z_NO_COMPRESSION || qualityAsLevel > Z_BEST_COMPRESSION)
			throw std::runtime_error("compression level (" + std::to_string(qualityAsLevel) + ") is out of bounds");

		auto zlib = ZlibCompressStream(qualityAsLevel, static_cast<std::underlying_type_t<Quality>>(header));
		std::vector<uint8_t> compressed;

		size_t cursor_in = 0;
		size_t cursor_out = 0;
		bool eob = false;
		int opres = Z_OK;

		do {

			eob = cursor_in + zlib.chunk >= input.size();
			zlib.stream.avail_in = eob ? input.size() - cursor_in : zlib.chunk;
			zlib.stream.next_in = (uint8_t*)(input.data() + cursor_in);
			cursor_in += zlib.stream.avail_in;

			do {

				zlib.stream.avail_out = zlib.chunk;
				if ((compressed.size() - cursor_out) < zlib.stream.avail_out)
					compressed.resize(compressed.size() + zlib.stream.avail_out);
				zlib.stream.next_out = compressed.data() + cursor_out;

				opres = deflate(&zlib.stream, eob ? Z_FINISH : Z_NO_FLUSH);
				if (opres < 0) throw std::runtime_error("deflate stream error: code " + std::to_string(opres));

				cursor_out += zlib.chunk - zlib.stream.avail_out;

			} while (zlib.stream.avail_out == 0);

		} while (!eob);

		if (opres != Z_STREAM_END) throw std::runtime_error("deflate stream failed to properly finish");

		compressed.resize(cursor_out);
		compressed.shrink_to_fit();
		
		return compressed;
		
	} catch(const std::exception& e) {
		throw std::runtime_error(std::string("zlibCompressBuffer failed: ") + e.what());
	}
}

std::vector<uint8_t> Compress::zlibDecompressBuffer(const std::vector<uint8_t>& input) {

	if (!input.size()) return {};

	static const auto winbitOpenAuto = 32;

	try {

		auto zlib = ZlibDecompressStream(winbitOpenAuto);
		std::vector<uint8_t> decompressed;

		size_t cursor_in = 0;
		size_t cursor_out = 0;
		bool eob = false;
		int opres = Z_OK;

		do {

			eob = cursor_in + zlib.chunk >= input.size();
			zlib.stream.avail_in = eob ? input.size() - cursor_in : zlib.chunk;
			zlib.stream.next_in = (uint8_t*)(input.data() + cursor_in);
			cursor_in += zlib.stream.avail_in;

			do {

				zlib.stream.avail_out = zlib.chunk;
				if ((decompressed.size() - cursor_out) < zlib.stream.avail_out)
					decompressed.resize(decompressed.size() + zlib.stream.avail_out);
				zlib.stream.next_out = decompressed.data() + cursor_out;

				opres = inflate(&zlib.stream, Z_NO_FLUSH);
				if (opres < Z_OK || opres > Z_STREAM_END) throw std::runtime_error("inflate stream error: code " + std::to_string(opres));

				cursor_out += zlib.chunk - zlib.stream.avail_out;

			} while (zlib.stream.avail_out == 0);

		} while (!eob && opres != Z_STREAM_END);

		if (opres != Z_STREAM_END) throw std::runtime_error("inflate stream failed to properly finish");

		decompressed.resize(cursor_out);
		decompressed.shrink_to_fit();
		
		return decompressed;

	} catch(const std::exception& e) {
		throw std::runtime_error(std::string("zlibDecompressBuffer failed: ") + e.what());
	}
}


GzipStreamCompressor::GzipStreamCompressor(GzipStreamCompressor&& other) {
	this->m_stage = other.m_stage;
	this->m_stream = other.m_stream;
	other.m_stream = nullptr;
}

GzipStreamCompressor& GzipStreamCompressor::operator=(GzipStreamCompressor&& other) {
	this->m_stage = other.m_stage;
	this->m_stream = other.m_stream;
	other.m_stream = nullptr;
	return *this;
}

GzipStreamCompressor::GzipStreamCompressor(Quality quality) {

	this->m_stream = new z_stream;
	memset(this->m_stream, 0, sizeof(z_stream));

	auto qualityAsInt = static_cast<std::underlying_type_t<Quality>>(quality);
	auto winbitsAsInt = static_cast<std::underlying_type_t<ZlibSetHeader>>(ZlibSetHeader::Gzip);
	auto initResult = deflateInit2(reinterpret_cast<z_stream*>(this->m_stream), qualityAsInt, Z_DEFLATED, winbitsAsInt, 8, Z_DEFAULT_STRATEGY);
	
	if (initResult != Z_OK) {
		throw std::runtime_error("Could not initialize deflate (zlib error code " + std::to_string(initResult) + ')');
	}
}

GzipStreamCompressor::~GzipStreamCompressor() {
	if (this->m_stream == nullptr) return;
	(void)inflateEnd(reinterpret_cast<z_stream*>(this->m_stream));
	delete reinterpret_cast<z_stream*>(this->m_stream);
}

std::vector<uint8_t> GzipStreamCompressor::nextChunk(std::vector<uint8_t>& next, StreamFlush flush) {

	z_stream* castedctx = reinterpret_cast<z_stream*>(this->m_stream);

	castedctx->avail_in = next.size();
	castedctx->next_in = next.data();

	std::array<uint8_t, this->chunkSize> tempBuff;
	std::vector<uint8_t> resultBuff;

	do {

		castedctx->avail_out = tempBuff.size();
		castedctx->next_out = tempBuff.data();

		auto resultStat = deflate(castedctx, flush == StreamFlush::Finish ? Z_NO_FLUSH : Z_FINISH);
		if (resultStat < 0) {
			throw std::runtime_error("deflate stream error " + std::to_string(resultStat));
		}

		auto sizeOut = tempBuff.size() - castedctx->avail_out;
		resultBuff.insert(resultBuff.end(), tempBuff.begin(), tempBuff.begin() + sizeOut);

	} while (castedctx->avail_out == 0);

	return resultBuff;

}

std::vector<uint8_t> GzipStreamCompressor::nextChunk(std::vector<uint8_t>& next) {
	return this->nextChunk(next, StreamFlush::Auto);
}

std::vector<uint8_t> GzipStreamCompressor::end() {

	z_stream* castedctx = reinterpret_cast<z_stream*>(this->m_stream);

	castedctx->avail_in = 0;
	castedctx->next_in = nullptr;

	std::array<uint8_t, this->chunkSize> tempBuff;
	std::vector<uint8_t> resultBuff;

	do {

		castedctx->avail_out = tempBuff.size();
		castedctx->next_out = tempBuff.data();

		auto resultStat = deflate(castedctx, Z_FINISH);
		if (resultStat < 0) {
			throw std::runtime_error("deflate stream error " + std::to_string(resultStat));
		}

		auto sizeOut = tempBuff.size() - castedctx->avail_out;
		resultBuff.insert(resultBuff.end(), tempBuff.begin(), tempBuff.begin() + sizeOut);

	} while (castedctx->avail_out == 0);

	return resultBuff;
}

void GzipStreamCompressor::reset() {

	//	I wanted to add some other stuff here but let's leave it all just is,
	//	having a switch here doesn't hurt anybody
	switch (this->m_stage) {

		case Stage::Ended: {
			deflateReset(reinterpret_cast<z_stream*>(this->m_stream));
		} break;

		case Stage::Progress: {
			deflateReset(reinterpret_cast<z_stream*>(this->m_stream));
		} break;

		default: break;
	}
}
