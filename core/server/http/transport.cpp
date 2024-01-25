
#include "../http.hpp"
#include "../../network/sysnetw.hpp"
#include "../../compression/compression.hpp"
#include "../../polyfill/polyfill.hpp"
#include "../../crypto/crypto.hpp"
#include "../../../lambda_build_options.hpp"

#include <queue>
#include <mutex>
#include <future>
#include <thread>
#include <algorithm>
#include <map>
#include <set>

using namespace Lambda;
using namespace Lambda::Network;
using namespace Lambda::Server;
using namespace Lambda::Server::HTTP;

static const std::string patternEndHeader = "\r\n\r\n";

static const std::map<ContentEncodings, std::string> contentEncodingMap = {
	{ ContentEncodings::Brotli, "br" },
	{ ContentEncodings::Gzip, "gzip" },
	{ ContentEncodings::Deflate, "deflate" },
};

void Server::HTTP::asyncReader(Network::TCP::Connection& conn, HttpRequestQueue& queue) {

}

void Server::HTTP::writeResponse(Lambda::HTTP::Response& response, Network::TCP::Connection& conn, ContentEncodings useEncoding) {

#ifdef LAMBDA_CONTENT_ENCODING_ENABLED

	std::vector<uint8_t> responseBody;

	switch (useEncoding) {

		case ContentEncodings::Brotli: {
			responseBody = Compress::brotliCompressBuffer(response.body.buffer(), Compress::Quality::Noice);
		} break;

		case ContentEncodings::Gzip: {
			responseBody = Compress::zlibCompressBuffer(response.body.buffer(), Compress::Quality::Noice, Compress::ZlibSetHeader::Gzip);
		} break;

		case ContentEncodings::Deflate: {
			responseBody = Compress::zlibCompressBuffer(response.body.buffer(), Compress::Quality::Noice, Compress::ZlibSetHeader::Defalte);
		} break;

		default: {
			responseBody = response.body.buffer();
		} break;
	}

	if (useEncoding != ContentEncodings::None) {
		response.headers.set("content-encoding", contentEncodingMap.at(useEncoding));
	}

#else
	auto& responseBody = response.body.buffer();
#endif

	auto bodySize = responseBody.size();
	response.headers.set("content-length", std::to_string(bodySize));

	std::string headerBuff = "HTTP/1.1 " + std::to_string(response.status.code()) + ' ' + response.status.text() + "\r\n";
	for (const auto& header : response.headers.entries()) {
		headerBuff += header.first + ": " + header.second + "\r\n";
	}
	headerBuff += "\r\n";

	conn.write(std::vector<uint8_t>(headerBuff.begin(), headerBuff.end()));
	if (bodySize) conn.write(responseBody);

}
