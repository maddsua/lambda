#ifndef _OCTOPUSS_EXTRA_COMPRESSION_STREAMS_
#define _OCTOPUSS_EXTRA_COMPRESSION_STREAMS_

#include <cstring>
#include <brotli/encode.h>
#include <brotli/decode.h>
#include <zlib.h>

#include <stdexcept>

namespace Compress {

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

	class ZlibStream {
		public:
			z_stream stream;

			ZlibStream() {
				memset(&this->stream, 0, sizeof(z_stream));
			}

			static const size_t chunk = (128 * 1024);
	};

	class ZlibCompressStream : public ZlibStream {
		public:
			ZlibCompressStream(int compression, int winbits) {
				auto initResult = deflateInit2(&this->stream, compression, Z_DEFLATED, winbits, 8, Z_DEFAULT_STRATEGY);
				if (initResult != Z_OK) throw std::runtime_error("Could not initialize deflate (zlib error code " + std::to_string(initResult) + ')');
			}
			~ZlibCompressStream() {
				(void)deflateEnd(&this->stream);
			}
	};

	class ZlibDecompressStream : public ZlibStream {
		public:
			ZlibDecompressStream(int winbits) {
				auto initResult = inflateInit2(&this->stream, winbits);
				if (initResult != Z_OK) throw std::runtime_error("Could not initialize inflate (zlib error code " + std::to_string(initResult) + ')');
			}
			~ZlibDecompressStream() {
				(void)inflateEnd(&this->stream);
			}
	};

};

#endif
