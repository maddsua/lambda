/*
	zlib and brotli classes
	2023 maddsua
	https://github.com/maddsua/
*/

#define CHUNK_128K	(131072)


#ifndef H_MADDSUA_LAMBDA_COMPRESSION
#define H_MADDSUA_LAMBDA_COMPRESSION

	#include <string>
	#include <vector>
	#include <stdint.h>
	#include <zlib.h>

	namespace lambda {

		class zlibDecompressStream {
			public:
				static const int winbit_auto = 32;
				static const int winbit_gzip = 16;
				static const size_t chunkSize = CHUNK_128K;

				zlibDecompressStream();
				~zlibDecompressStream();

				bool init(int windowBits);
				inline bool init() {
					return init(winbit_auto);
				}

				bool done();
				bool error();

				bool decompressChunk(uint8_t* bufferIn, size_t dataInSize, std::vector <uint8_t>* bufferOut);
			
			private:
				uint8_t* tempBuffer;
				z_stream* datastream = nullptr;
				bool initialized = false;
				int streamStatus = Z_OK;
		};

		class zlibCompressStream {
			public:
				static const size_t chunkSize = CHUNK_128K;
				static const int expect_ratio = 3;
				static const int defCompress = Z_DEFAULT_COMPRESSION;
				static const int header_gz = 26;
				static const int header_deflate = 8;
				static const int header_raw = -15;

				zlibCompressStream();
				~zlibCompressStream();

				bool init(int compression, int header);

				bool done();
				bool error();

				bool compressChunk(uint8_t* bufferIn, size_t dataInSize, std::vector <uint8_t>* bufferOut, bool finish);

			private:
				uint8_t* tempBuffer;
				z_stream* datastream = nullptr;
				bool initialized = false;
				int streamStatus = Z_OK;

				static const int def_memlvl = 9;
		};

		class brotliCompressStream {
			public:
				static const size_t chunkSize = CHUNK_128K;
				static const int expect_ratio = 5;

				brotliCompressStream();
				~brotliCompressStream();

				bool setQuality(int quality);
				bool done();

				bool compressChunk(const uint8_t* chunk, const size_t chunkSize, std::vector <uint8_t>* bufferOut, bool finish);

			private:
				void* instance;
		};

		class brotliDecompressStream {
			public:
				static const size_t chunkSize = CHUNK_128K;

				brotliDecompressStream();
				~brotliDecompressStream();

				bool done();

				bool decompressChunk(const uint8_t* chunk, const size_t chunkSize, std::vector <uint8_t>* bufferOut);

			private:
				void* instance;
		};

		std::string gzCompress(const std::string* data, bool gzipHeader);
		std::string gzDecompress(const std::string* data);

		std::string brCompress(const std::string* data);
		std::string brDecompress(const std::string* data);
	}

#endif