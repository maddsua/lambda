/*
	zlib and brotli classes
	2023 maddsua
	https://github.com/maddsua/
*/


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
				static const size_t chunkSize = 131072;

				zlibDecompressStream();
				~zlibDecompressStream();

				bool init(int windowBits);
				inline bool init() {
					return init(winbit_auto);
				}

				bool done();
				bool error();

				bool doInflate(uint8_t* bufferIn, size_t dataInSize, std::vector <uint8_t>* bufferOut);
			
			private:
				uint8_t* tempBuffer;
				z_stream* datastream = nullptr;
				bool initialized = false;
				int streamStatus = Z_OK;
		};

		class zlibCompressStream {
			public:
				static const size_t chunkSize = 131072;
				static const int expect_ratio = -15;
				static const int header_gz = 26;
				static const int header_deflate = 8;
				static const int header_raw = -15;

				zlibCompressStream();
				~zlibCompressStream();

				bool init(int compression, int header);

				bool done();
				bool error();

				bool doDeflate(uint8_t* bufferIn, size_t dataInSize, std::vector <uint8_t>* bufferOut, bool flush);

			private:
				uint8_t* tempBuffer;
				z_stream* datastream = nullptr;
				bool initialized = false;
				int streamStatus = Z_OK;

				static const int def_memlvl = 9;
		};

		std::string gzCompress(const std::string* data, bool gzipHeader);
		std::string gzDecompress(const std::string* data);

		std::string brCompress(const std::string* data);
		//std::string brDecompress(const std::string* data);
	}

#endif