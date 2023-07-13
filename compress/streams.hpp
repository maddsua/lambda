#ifndef __LAMBDA_COMPRESSION_STREAMS__
#define __LAMBDA_COMPRESSION_STREAMS__

#include <cstring>
#include "../deps/brotli/c/include/brotli/encode.h"
#include "../deps/brotli/c/include/brotli/decode.h"
#include "../deps/zlib/zlib.h"

namespace Lambda::Compress {

	class BrotliCompressStream {
		public:
			BrotliEncoderState* stream = nullptr;
			static const size_t chunk = (128 * 1024);

			BrotliCompressStream() {
				this->stream = BrotliEncoderCreateInstance(nullptr, nullptr, nullptr);
			}
			~BrotliCompressStream() {
				BrotliEncoderDestroyInstance(this->stream);
			}
	};

	class BrotliDecompressStream {
		public:
			BrotliDecoderState* stream = nullptr;
			static const size_t chunk = (128 * 1024);

			BrotliDecompressStream() {
				this->stream = BrotliDecoderCreateInstance(nullptr, nullptr, nullptr);
			}
			~BrotliDecompressStream() {
				BrotliDecoderDestroyInstance(this->stream);
			}
	};

	class ZlibCompressStream {
		public:
			z_stream* stream = nullptr;
			static const size_t chunk = (128 * 1024);

			ZlibCompressStream(int compression, int winbits) {
				this->stream = new z_stream;
				memset(this->stream, 0, sizeof(z_stream));
				auto initResult = deflateInit2(stream, compression, Z_DEFLATED, winbits, 8, Z_DEFAULT_STRATEGY);
				if (initResult != Z_OK) throw Lambda::Error("Could not initialize deflate", initResult);
			}
			~ZlibCompressStream() {
				(void)deflateEnd(this->stream);
				delete this->stream;
			}
	};

	class ZlibDecompressStream {
		public:
			z_stream* stream = nullptr;
			static const size_t chunk = (128 * 1024);

			ZlibDecompressStream(int winbits) {
				this->stream = new z_stream;
				memset(this->stream, 0, sizeof(z_stream));
				auto initResult = inflateInit2(stream, winbits);
				if (initResult != Z_OK) throw Lambda::Error("Could not initialize inflate", initResult);
			}
			~ZlibDecompressStream() {
				(void)inflateEnd(this->stream);
				delete this->stream;
			}
	};

};

#endif
