#include "./compress.hpp"
#include <zlib.h>

using namespace Lambda;

Compress::ZlibStream::ZlibStream() {
	compressStatus = Z_OK;
	decompressStatus = Z_OK;
}
Compress::ZlibStream::~ZlibStream() {
	if (compressStream != nullptr) {
		(void)deflateEnd((z_stream*)compressStream);
		delete ((z_stream*)compressStream);
	}
	if (compressTemp != nullptr) delete[] compressTemp;

	if (decompressStream != nullptr) {
		(void)inflateEnd((z_stream*)decompressStream);
		delete ((z_stream*)decompressStream);
	}
	if (decompressTemp != nullptr) delete[] decompressTemp;
}

bool Compress::ZlibStream::startCompression(int compression, int header) {

	if (compressStream != nullptr || compressTemp != nullptr) return false;

	if (compression < Z_NO_COMPRESSION || compression > 9) compression = -1;
	if (header != header_gz && header != header_deflate && header != header_raw) header = header_gz;

	compressStream = new z_stream;
	memset(compressStream, 0, sizeof(z_stream));

	compressTemp = new uint8_t[chunkSize];

	compressStatus = deflateInit2((z_stream*)compressStream, compression, Z_DEFLATED, header, 8, Z_DEFAULT_STRATEGY);
	return compressStatus == Z_OK;
}
bool Compress::ZlibStream::startCompression(int compression) {
	return startCompression(compression, header_gz);
}
bool Compress::ZlibStream::startCompression() {
	return startCompression(Z_DEFAULT_COMPRESSION, header_gz);
}

bool Compress::ZlibStream::compressionDone() {
	return compressStatus == Z_STREAM_END;
}

bool Compress::ZlibStream::compressionError() {
	return (compressStatus < Z_OK || compressStatus == Z_NEED_DICT);
}

int Compress::ZlibStream::compressionStatus() {
	return compressStatus;
}

bool Compress::ZlibStream::compressChunk(uint8_t* bufferIn, size_t dataInSize, std::vector <uint8_t>* bufferOut, bool finish) {

	if (compressStream == nullptr || compressTemp == nullptr) return false;

	auto instance = ((z_stream*)compressStream);

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

bool Compress::ZlibStream::compressBuffer(std::vector<uint8_t>* bufferIn, std::vector<uint8_t>* bufferOut) {

	if (compressStream == nullptr || compressTemp == nullptr) return false;

	if (!bufferIn->size()) return false;

	bufferOut->clear();
	bufferOut->reserve(bufferIn->size() / expect_ratio);
	
	bool streamEnd = false;
	size_t dataProcessed = 0;
	size_t partSize = 0;

	do {
		streamEnd = (dataProcessed + chunkSize) >= bufferIn->size();
		partSize = streamEnd ? (bufferIn->size() - dataProcessed) : chunkSize;

		compressChunk(bufferIn->data() + dataProcessed, partSize, bufferOut, streamEnd);
		if (compressionError()) return false;
		dataProcessed += partSize;
	} while (!streamEnd && !compressionError());

	//	return empty string on error
	if (!compressionDone() || compressionError()) return false;

	return true;
}

bool Compress::ZlibStream::compressionReset() {
	compressStatus = inflateReset((z_stream*)compressStream);
	return compressStatus == Z_OK;
}


bool Compress::ZlibStream::startDecompression(int wbits) {

	if (decompressStream != nullptr || decompressStream != nullptr) return false;

	decompressStream = new z_stream;
	memset(decompressStream, 0, sizeof(z_stream));

	decompressTemp = new uint8_t[chunkSize];

	decompressStatus = inflateInit2((z_stream*)decompressStream, wbits);
	return decompressStatus == Z_OK;
}
bool Compress::ZlibStream::startDecompression() {
	return startDecompression(winbit_auto);
}

bool Compress::ZlibStream::decompressionDone() {
	return decompressStatus == Z_STREAM_END;
}

bool Compress::ZlibStream::decompressionError() {
	return (decompressStatus < Z_OK || compressStatus == Z_NEED_DICT);
}

int Compress::ZlibStream::decompressionStatus() {
	return decompressStatus;
}

bool Compress::ZlibStream::decompressChunk(uint8_t* bufferIn, size_t dataInSize, std::vector <uint8_t>* bufferOut) {

	if (decompressStream == nullptr || decompressTemp == nullptr) return false;

	auto instance = ((z_stream*)decompressStream);

	instance->next_in = bufferIn;
	instance->avail_in = dataInSize;

	do {
		instance->avail_out = chunkSize;
		instance->next_out = decompressTemp;
		
		decompressStatus = inflate(instance, Z_NO_FLUSH);
		if (decompressionError()) return false;

		bufferOut->insert(bufferOut->end(), decompressTemp, decompressTemp + (chunkSize - instance->avail_out));
		
	} while (instance->avail_out == 0);
	
	return !decompressionError();
}

bool Compress::ZlibStream::decompressBuffer(std::vector<uint8_t>* bufferIn, std::vector<uint8_t>* bufferOut) {

	if (decompressStream == nullptr || decompressTemp == nullptr) return false;

	if (!bufferIn->size()) return false;

	bufferOut->clear();
	bufferOut->reserve(bufferIn->size() * expect_ratio);

	bool streamEnd = false;
	size_t dataProcessed = 0;
	size_t partSize = 0;

	do {
		streamEnd = (dataProcessed + chunkSize) >= bufferIn->size();
		partSize = streamEnd ? (bufferIn->size() - dataProcessed) : chunkSize;

		decompressChunk(bufferIn->data() + dataProcessed, partSize, bufferOut);
		if (decompressionError()) return false;
		dataProcessed += partSize;

	} while (!streamEnd && !decompressionError());

	//	return empty string on error
	if (!decompressionDone() || decompressionError()) return false;

	return true;
}

bool Compress::ZlibStream::decompressionReset() {
	decompressStatus = deflateReset((z_stream*)decompressStream);
	return decompressStatus == Z_OK;
}
