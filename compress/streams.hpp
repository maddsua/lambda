#ifndef __LAMBDA_COMPRESSION_STREAMS__
#define __LAMBDA_COMPRESSION_STREAMS__

#include <brotli/encode.h>
#include <brotli/decode.h>
#include <zlib.h>

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

#endif
