#include "./compress.hpp"
#include <zlib.h>

Lambda::Compress::ZlibStream::ZlibStream() {
	compressStatus = Z_OK;
	decompressStatus = Z_OK;
}
Lambda::Compress::ZlibStream::~ZlibStream() {
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

bool Lambda::Compress::ZlibStream::startCompression(int compression, int header) {

	if (compressStream != nullptr || compressTemp != nullptr) return false;

	if (compression < Z_NO_COMPRESSION || compression > 9) compression = -1;
	if (header != header_gz && header != header_deflate && header != header_raw) header = header_gz;

	compressStream = new z_stream;
	compressTemp = new uint8_t[chunkSize];

	compressStatus = deflateInit2((z_stream*)compressStream, compression, Z_DEFLATED, header, 9, Z_DEFAULT_STRATEGY);
	return compressStatus == Z_OK;
}
bool Lambda::Compress::ZlibStream::startCompression(int compression) {
	return startCompression(compression, header_gz);
}
bool Lambda::Compress::ZlibStream::startCompression() {
	return startCompression(Z_DEFAULT_COMPRESSION, header_gz);
}

bool Lambda::Compress::ZlibStream::compressionDone() {
	return compressStatus == Z_STREAM_END;
}

bool Lambda::Compress::ZlibStream::compressionError() {
	return (compressStatus < Z_OK || compressStatus == Z_NEED_DICT);
}

int Lambda::Compress::ZlibStream::compressionStatus() {
	return compressStatus;
}

bool Lambda::Compress::ZlibStream::compressChunk(uint8_t* bufferIn, size_t dataInSize, std::vector <uint8_t>* bufferOut, bool finish) {

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

bool Lambda::Compress::ZlibStream::compressBuffer(std::vector<uint8_t>* bufferIn, std::vector<uint8_t>* bufferOut) {

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

bool Lambda::Compress::ZlibStream::compressionReset() {
	compressStatus = inflateReset((z_stream*)compressStream);
	return compressStatus == Z_OK;
}


bool Lambda::Compress::ZlibStream::startDecompression(int wbits) {

	decompressStream = new z_stream;
	decompressTemp = new uint8_t[chunkSize];

	decompressStatus = inflateInit2((z_stream*)decompressStream, wbits);
	return decompressStatus == Z_OK;
}
bool Lambda::Compress::ZlibStream::startDecompression() {
	return startDecompression(winbit_auto);
}

bool Lambda::Compress::ZlibStream::decompressionDone() {
	return decompressStatus == Z_STREAM_END;
}

bool Lambda::Compress::ZlibStream::decompressionError() {
	return (decompressStatus < Z_OK || compressStatus == Z_NEED_DICT);
}

int Lambda::Compress::ZlibStream::decompressionStatus() {
	return decompressStatus;
}

bool Lambda::Compress::ZlibStream::decompressChunk(uint8_t* bufferIn, size_t dataInSize, std::vector <uint8_t>* bufferOut) {

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

bool Lambda::Compress::ZlibStream::decompressBuffer(std::vector<uint8_t>* bufferIn, std::vector<uint8_t>* bufferOut) {

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

bool Lambda::Compress::ZlibStream::decompressionReset() {
	decompressStatus = deflateReset((z_stream*)decompressStream);
	return decompressStatus == Z_OK;
}