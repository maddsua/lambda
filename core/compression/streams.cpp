#include "./streams.hpp"

using namespace Lambda::Compress;


BrotliCompressStream::BrotliCompressStream() {
	this->stream = BrotliEncoderCreateInstance(nullptr, nullptr, nullptr);
}
BrotliCompressStream::~BrotliCompressStream() {
	BrotliEncoderDestroyInstance(this->stream);
}

BrotliDecompressStream::BrotliDecompressStream() {
	this->stream = BrotliDecoderCreateInstance(nullptr, nullptr, nullptr);
}
BrotliDecompressStream::~BrotliDecompressStream() {
	BrotliDecoderDestroyInstance(this->stream);
}

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
