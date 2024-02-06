#ifndef __LIB_MADDSUA_LAMBDA_CORE_HTTP_TRANSPORT__
#define __LIB_MADDSUA_LAMBDA_CORE_HTTP_TRANSPORT__

#include "../network/tcp/connection.hpp"
#include "./http.hpp"

#include <vector>
#include <string>
#include <unordered_map>
#include <map>
#include <optional>

namespace Lambda::HTTP::Transport {

	struct TransportOptions {
		bool useCompression = true;
		bool reuseConnections = true;
		size_t maxRequestSize = 25 * 1024 * 1024;
	};

	enum struct ContentEncodings {
		None, Brotli, Gzip, Deflate,
	};

	class V1TransportContext {
		private:
			Network::TCP::Connection& conn;
			const TransportOptions& opts;
			std::vector<uint8_t> readbuffer;
			bool keepAlive = false;
			ContentEncodings acceptsEncoding = ContentEncodings::None;

		public:
			V1TransportContext(Network::TCP::Connection& connInit, const TransportOptions& optsInit);

			V1TransportContext(const V1TransportContext& other) = delete;
			V1TransportContext& operator=(const V1TransportContext& other) = delete;

			std::optional<HTTP::Request> nextRequest();
			void respond(const HTTP::Response& response);
	};
};

#endif
