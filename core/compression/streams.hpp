#ifndef  __LIB_MADDSUA_LAMBDA_EXTRA_COMPRESSION_STREAMS__
#define  __LIB_MADDSUA_LAMBDA_EXTRA_COMPRESSION_STREAMS__

#include <cstring>
#include <cstdint>
#include <brotli/encode.h>
#include <brotli/decode.h>
#include <zlib.h>

#include <stdexcept>

namespace Lambda::Compress::Streams {

	static const size_t defaultChunkSize = 128 * 1024;

	struct BrotliCompressStream {
		BrotliEncoderState* stream = nullptr;
		static const size_t chunk = defaultChunkSize;

		BrotliCompressStream();
		~BrotliCompressStream();
	};

	struct BrotliDecompressStream {
		BrotliDecoderState* stream = nullptr;
		static const size_t chunk = defaultChunkSize;

		BrotliDecompressStream();
		~BrotliDecompressStream();
	};

	struct ZlibStream {
		z_stream stream;
		static const size_t chunk = defaultChunkSize;

		ZlibStream();
	};

	struct ZlibCompressStream : ZlibStream {
		ZlibCompressStream(int compression, int winbits);
		~ZlibCompressStream();
	};

	struct ZlibDecompressStream : ZlibStream {
		ZlibDecompressStream(int winbits);
		~ZlibDecompressStream();
	};
};

#endif
