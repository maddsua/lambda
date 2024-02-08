#include "./compression.hpp"

#include <cstring>
#include <zlib.h>
#include <stdexcept>
#include <array>

using namespace Lambda;
using namespace Lambda::Compress;

GzipStreamDecompressor::GzipStreamDecompressor(GzipStreamDecompressor&& other) {
	this->m_stage = other.m_stage;
	this->m_stream = other.m_stream;
	other.m_stream = nullptr;
}

GzipStreamDecompressor& GzipStreamDecompressor::operator=(GzipStreamDecompressor&& other) {
	this->m_stage = other.m_stage;
	this->m_stream = other.m_stream;
	other.m_stream = nullptr;
	return *this;
}

GzipStreamDecompressor::GzipStreamDecompressor(Quality quality) {

	this->m_stream = new z_stream;
	memset(this->m_stream, 0, sizeof(z_stream));

	auto qualityAsInt = static_cast<std::underlying_type_t<Quality>>(quality);
	auto winbitsAsInt = static_cast<std::underlying_type_t<ZlibSetHeader>>(ZlibSetHeader::Gzip);
	auto initResult = deflateInit2(reinterpret_cast<z_stream*>(this->m_stream), qualityAsInt, Z_DEFLATED, winbitsAsInt, 8, Z_DEFAULT_STRATEGY);
	
	if (initResult != Z_OK) {
		throw std::runtime_error("Could not initialize deflate (zlib error code " + std::to_string(initResult) + ')');
	}
}

GzipStreamDecompressor::~GzipStreamDecompressor() {
	if (this->m_stream == nullptr) return;
	(void)inflateEnd(reinterpret_cast<z_stream*>(this->m_stream));
	delete reinterpret_cast<z_stream*>(this->m_stream);
}


std::vector<uint8_t> GzipStreamDecompressor::nextChunk(std::vector<uint8_t>& next) {

	if (this->m_stage == Stage::Done) {
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

		auto resultStat = inflate(castedctx, Z_NO_FLUSH);
		if (resultStat < 0) {
			throw std::runtime_error("deflate stream error " + std::to_string(resultStat));
		}

		if (resultStat == Z_STREAM_END) {
			this->m_stage = Stage::Done;
		}

		auto sizeOut = tempBuff.size() - castedctx->avail_out;
		resultBuff.insert(resultBuff.end(), tempBuff.begin(), tempBuff.begin() + sizeOut);

	} while (castedctx->avail_out == 0);

	return resultBuff;
}

bool GzipStreamDecompressor::isDone() const noexcept {
	this->m_stage == Stage::Done;
}

void GzipStreamDecompressor::reset() noexcept {

	//	I wanted to add some other stuff here but let's leave it all just is,
	//	having a switch here doesn't hurt anybody
	switch (this->m_stage) {

		case Stage::Done: {
			deflateReset(reinterpret_cast<z_stream*>(this->m_stream));
		} break;

		case Stage::Progress: {
			deflateReset(reinterpret_cast<z_stream*>(this->m_stream));
		} break;

		default: break;
	}

	this->m_stage = Stage::Ready;
}
