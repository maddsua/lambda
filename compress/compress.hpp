#ifndef __LAMBDA_COMPRESSION__
#define __LAMBDA_COMPRESSION__

#include <stdint.h>
#include <vector>

namespace Lambda::Compress {

	class ZlibStream {
		private:
			void* compressStream = nullptr;
			int compressStatus;
			uint8_t* compressTemp = nullptr;

			void* decompressStream = nullptr;
			int decompressStatus;
			uint8_t* decompressTemp = nullptr;

		public:
			ZlibStream();
			~ZlibStream();

			bool startCompression(int compression, int header);
			bool startCompression(int compression);
			bool startCompression();
			bool compressionDone();
			bool compressionError();
			int compressionStatus();
			bool compressChunk(uint8_t* bufferIn, size_t dataInSize, std::vector <uint8_t>* bufferOut, bool finish);
			bool compressBuffer(std::vector<uint8_t>* bufferIn, std::vector<uint8_t>* bufferOut);
			bool compressionReset();

			bool startDecompression(int wbits);
			bool startDecompression();
			bool decompressionDone();
			bool decompressionError();
			int decompressionStatus();
			bool decompressChunk(uint8_t* bufferIn, size_t dataInSize, std::vector <uint8_t>* bufferOut);
			bool decompressBuffer(std::vector<uint8_t>* bufferIn, std::vector<uint8_t>* bufferOut);
			bool decompressionReset();

			static const size_t chunkSize = 131072;
			static const int expect_ratio = 3;
			static const int header_gz = 26;
			static const int header_deflate = 8;
			static const int header_raw = -15;
			static const int winbit_auto = 32;
			static const int winbit_gzip = 16;
	};

	class BrotliStream {
		private:
			void* compressStream = nullptr;
			int compressStatus;
			void* decompressStream = nullptr;
			int decompressStatus;

		public:
			BrotliStream();
			~BrotliStream();

			bool startCompression(int quality);
			bool startCompression();
			bool compressionDone();
			bool compressionError();
			int compressionStatus();
			bool compressChunk(const uint8_t* bufferIn, size_t dataInSize, std::vector <uint8_t>* bufferOut, bool finish);
			bool compressBuffer(const std::vector<uint8_t>* bufferIn, std::vector<uint8_t>* bufferOut);
			bool compressionReset();

			bool startDecompression();
			bool decompressionDone();
			bool decompressionError();
			int decompressionStatus();
			bool decompressChunk(const uint8_t* bufferIn, size_t dataInSize, std::vector <uint8_t>* bufferOut);
			bool decompressBuffer(const std::vector<uint8_t>* bufferIn, std::vector<uint8_t>* bufferOut);
			bool decompressionReset();

			static const size_t chunkSize = 131072;
			static const int expect_ratio = 4;
	};

};

#endif