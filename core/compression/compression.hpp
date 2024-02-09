#ifndef __LIB_MADDSUA_LAMBDA_EXTRA_COMPRESSION__
#define __LIB_MADDSUA_LAMBDA_EXTRA_COMPRESSION__

#include <cstdint>
#include <cstddef>
#include <vector>

namespace Lambda::Compress {

	static const size_t defaultChunkSize = 128 * 1024;

	enum struct Quality {
		Store = 0,
		Barely = 1,
		Fast = 2,
		Reasonable = 3,
		Good = 4,
		Better = 5,
		Noice = 6,
		Real_noice = 7,
		Squishin = 8,
		Max = 9,
	};

	std::vector<uint8_t> brotliCompressBuffer(const std::vector<uint8_t>& input, Quality quality);
	std::vector<uint8_t> brotliDecompressBuffer(const std::vector<uint8_t>& input);

	enum struct ZlibSetHeader {
		Gzip = 26,
		Defalte = 8,
		Raw = -15,
	};

	std::vector<uint8_t> zlibCompressBuffer(const std::vector<uint8_t>& input, Quality quality, ZlibSetHeader header);
	std::vector<uint8_t> zlibDecompressBuffer(const std::vector<uint8_t>& input);

	class GzipStreamCompressor {
		private:

			enum struct Stage {
				Ready, Progress, Ended
			};

			void* m_stream = nullptr;
			Stage m_stage = Stage::Ready;

		public:

			enum struct StreamFlush {
				Noflush, Finish
			};

			GzipStreamCompressor(Quality quality);
			GzipStreamCompressor(const GzipStreamCompressor&) = delete;
			GzipStreamCompressor(GzipStreamCompressor&& other);
			~GzipStreamCompressor();

			GzipStreamCompressor& operator=(const GzipStreamCompressor& other) = delete;
			GzipStreamCompressor& operator=(GzipStreamCompressor&& other);

			std::vector<uint8_t> nextChunk(std::vector<uint8_t>& next);
			std::vector<uint8_t> nextChunk(std::vector<uint8_t>& next, StreamFlush flush);
			std::vector<uint8_t> end();
			void reset();

			static const size_t chunkSize = 128 * 1024;
	};

	class GzipStreamDecompressor {
		private:

			enum struct Stage {
				Ready, Progress, Done
			};

			void* m_stream = nullptr;
			Stage m_stage = Stage::Ready;

		public:
			GzipStreamDecompressor();
			GzipStreamDecompressor(const GzipStreamDecompressor&) = delete;
			GzipStreamDecompressor(GzipStreamDecompressor&& other);
			~GzipStreamDecompressor();

			GzipStreamDecompressor& operator=(const GzipStreamDecompressor& other) = delete;
			GzipStreamDecompressor& operator=(GzipStreamDecompressor&& other) noexcept;

			std::vector<uint8_t> nextChunk(std::vector<uint8_t>& next);
			bool isDone() const noexcept;
			void reset() noexcept;

			static const size_t chunkSize = 128 * 1024;
	};
};

#endif
