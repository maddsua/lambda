#include "compression.hpp"
#include <zlib.h>

Compress::ZlibStream::ZlibStream() {
	this->compressStatus = Z_OK;
	this->decompressStatus = Z_OK;
}
Compress::ZlibStream::~ZlibStream() {
	if (this->compressStream != nullptr)
		delete this->compressStream;
	if (this->compressTemp != nullptr)
		delete[] this->compressTemp;

	if (this->decompressStream != nullptr)
		delete this->decompressStream;
	if (this->decompressTemp != nullptr)
		delete[] this->decompressTemp;
}

bool Compress::ZlibStream::startCompression(int compression, int header) {

	if (this->compressStream != nullptr || this->compressTemp != nullptr) return false;

	if (compression < 0 || compression > 9) compression = -1;
	if (header != this->header_gz && header != this->header_deflate && header != this->header_raw) header = this->header_gz;

	this->compressStream = new z_stream;
	this->compressTemp = new uint8_t[chunkSize];

	compressStatus = deflateInit2((z_stream*)compressStream, compression, Z_DEFLATED, header, 9, Z_DEFAULT_STRATEGY);
	return compressStatus == Z_OK;
}
bool Compress::ZlibStream::startCompression(int compression) {
	return startCompression(compression, this->header_gz);
}
bool Compress::ZlibStream::startCompression() {
	return startCompression(Z_DEFAULT_COMPRESSION, this->header_gz);
}

bool Compress::ZlibStream::compressionDone() {
	return compressStatus == Z_STREAM_END;
}

bool Compress::ZlibStream::compressionError() {
	return (compressStatus < Z_OK || compressStatus == Z_NEED_DICT);
}

bool Compress::ZlibStream::compressChunk(uint8_t* bufferIn, size_t dataInSize, std::vector <uint8_t>* bufferOut, bool finish) {

	z_stream* instance = ((z_stream*)compressStream);

	instance->next_in = bufferIn;
	instance->avail_in = dataInSize;

	do {
		instance->avail_out = chunkSize;
		instance->next_out = compressTemp;
		
		compressStatus = deflate(instance, finish ? Z_FINISH : Z_NO_FLUSH);
		if (compressionError()) return false;

		bufferOut->insert(bufferOut->end(), compressTemp, compressTemp + (chunkSize - instance->avail_out));
		
	} while (instance->avail_out == 0);
	
	return !compressionError();
}


bool Compress::ZlibStream::startDecompression(int wbits) {

	this->decompressStream = new z_stream;
	this->decompressTemp = new uint8_t[chunkSize];

	this->decompressStatus = inflateInit2((z_stream*)decompressStream, wbits);
	return decompressStatus == Z_OK;
}
bool Compress::ZlibStream::startDecompression(int wbits) {
	return startDecompression(this->winbit_auto);
}

bool Compress::ZlibStream::decompressionDone() {
	return decompressStatus == Z_STREAM_END;
}

bool Compress::ZlibStream::decompressionError() {
	return (decompressStatus < Z_OK || compressStatus == Z_NEED_DICT);
}

bool Compress::ZlibStream::decompressChunk(uint8_t* bufferIn, size_t dataInSize, std::vector <uint8_t>* bufferOut) {

	z_stream* instance = ((z_stream*)decompressStream);

	instance->next_in = bufferIn;
	instance->avail_in = dataInSize;

	do {
		instance->avail_out = chunkSize;
		instance->next_out = this->decompressTemp;
		
		decompressStatus = inflate(instance, Z_NO_FLUSH);
		if (decompressionError()) return false;

		bufferOut->insert(bufferOut->end(), this->decompressTemp, this->decompressTemp + (chunkSize - instance->avail_out));
		
	} while (instance->avail_out == 0);
	
	return !decompressionError();
}
