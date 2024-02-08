#include "./compression.hpp"

#include <cstring>
#include <zlib.h>
#include <stdexcept>
#include <array>

using namespace Lambda;
using namespace Lambda::Compress;

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

	if (this->m_stage == Stage::Ended) {
		throw std::runtime_error("Cannot push nextChunk to a stream that has been ended");
	}

	this->m_stage = Stage::Progress;

	z_stream* castedctx = reinterpret_cast<z_stream*>(this->m_stream);

	castedctx->avail_in = next.size();
	castedctx->next_in = next.data();

	std::array<uint8_t, this->chunkSize> tempBuff;
	std::vector<uint8_t> resultBuff;

	do {

		castedctx->avail_out = tempBuff.size();
		castedctx->next_out = tempBuff.data();

		auto resultStat = deflate(castedctx, flush == StreamFlush::Finish ? Z_FINISH : Z_NO_FLUSH);
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

	if (this->m_stage != Stage::Progress) {
		throw std::runtime_error("Cannot end a stream that has been already ended");
	}

	this->m_stage = Stage::Ended;

	z_stream* castedctx = reinterpret_cast<z_stream*>(this->m_stream);

	castedctx->avail_in = 0;
	castedctx->next_in = nullptr;

	std::array<uint8_t, this->chunkSize> tempBuff;
	std::vector<uint8_t> resultBuff;

	do {

		castedctx->avail_out = tempBuff.size();
		castedctx->next_out = tempBuff.data();

		auto resultStat = deflate(castedctx, Z_FINISH);

		switch (resultStat) {
			case Z_ERRNO: throw std::runtime_error("inflate error: os error " + std::to_string(errno));
			case Z_STREAM_ERROR: throw std::runtime_error("inflate error: stream error");
			case Z_DATA_ERROR: throw std::runtime_error("inflate error: data error");
			case Z_MEM_ERROR: throw std::runtime_error("inflate error: insufficient memory");
			case Z_BUF_ERROR: throw std::runtime_error("inflate error: buffer error");
			case Z_VERSION_ERROR: throw std::runtime_error("inflate error: unsupported version");
			default: break;
		}

		const auto sizeOut = tempBuff.size() - castedctx->avail_out;
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

	this->m_stage = Stage::Ready;
}
