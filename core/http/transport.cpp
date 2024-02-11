#include "./transport.hpp"
#include "../polyfill/polyfill.hpp"
#include "../compression/compression.hpp"


using namespace Lambda;
using namespace Lambda::HTTP;
using namespace Lambda::HTTP::Transport;

std::vector<uint8_t> Transport::compressContent(const std::vector<uint8_t>& data, ContentEncodings encoding) {
	switch (encoding) {

		case ContentEncodings::Brotli:
			return Compress::brotliCompressBuffer(data, Compress::Quality::Noice);

		case ContentEncodings::Gzip:
			return Compress::zlibCompressBuffer(data, Compress::Quality::Noice, Compress::ZlibSetHeader::Gzip);

		case ContentEncodings::Deflate:
			return Compress::zlibCompressBuffer(data, Compress::Quality::Noice, Compress::ZlibSetHeader::Defalte);

		default: return data;
	}
}
