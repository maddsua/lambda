#ifndef  __LIB_MADDSUA_LAMBDA_EXTRA_COMPRESSION_STREAMS__
#define  __LIB_MADDSUA_LAMBDA_EXTRA_COMPRESSION_STREAMS__

#include "../utils/utils.hpp"

#include <cstring>
#include <brotli/encode.h>
#include <brotli/decode.h>
#include <zlib.h>

#include <stdexcept>

namespace Lambda::Compress::Streams {

	using namespace Lambda::Literals;

	class BrotliCompressStream {
		public:
			BrotliEncoderState* stream = nullptr;
			static const size_t chunk = 128_KB;

			BrotliCompressStream();
			~BrotliCompressStream();
	};

	class BrotliDecompressStream {
		public:
			BrotliDecoderState* stream = nullptr;
			static const size_t chunk = 128_KB;

			BrotliDecompressStream();
			~BrotliDecompressStream();
	};

	class ZlibStream {
		public:
			z_stream stream;
			static const size_t chunk = 128_KB;

			ZlibStream();

	};

	class ZlibCompressStream : public ZlibStream {
		public:
			ZlibCompressStream(int compression, int winbits);
			~ZlibCompressStream();
	};

	class ZlibDecompressStream : public ZlibStream {
		public:
			ZlibDecompressStream(int winbits);
			~ZlibDecompressStream();
	};

};

#endif
