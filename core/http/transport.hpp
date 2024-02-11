#ifndef __LIB_MADDSUA_LAMBDA_CORE_HTTP_TRANSPORT__
#define __LIB_MADDSUA_LAMBDA_CORE_HTTP_TRANSPORT__

#include "../network/tcp/connection.hpp"
#include "./http.hpp"
#include "../utils/utils.hpp"

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

	class ProtocolError : public Lambda::Error {
		public:

			/**
			 * Originally this class had a variable to hold required action (drop request or respond with an error),
			 * but it was replaced with this optional as they provide the same logic and it doesn't really make sence
			 * to keep them both.
			 * 
			 * Side note, as of version 2 this error is only needed to indicate a request that is too large,
			 * for other errors like malformed headers we can just drop the connection.
			 * 
			 * So if you want to check if it's an error that we may want to
			 * handle with returning an error page  - check this optional for having a value
			*/
			const std::optional<HTTP::Status> respondStatus;

			ProtocolError(const std::string& message) : Error(message) {}
			ProtocolError(const std::string& message, HTTP::Status respondWithStatus) : Error(message), respondStatus(respondWithStatus) {}
	};

	struct TransportFlags {
		bool forceContentLength = true;
		bool autocompress = true;
	};

	class TransportContextV1 {
		private:
			Network::TCP::Connection& m_conn;
			const TransportOptions& m_topts;
			std::vector<uint8_t> m_readbuff;
			bool m_keepalive = false;
			ContentEncodings m_compress = ContentEncodings::None;

		public:
			TransportContextV1(Network::TCP::Connection& connInit, const TransportOptions& optsInit);

			TransportContextV1(const TransportContextV1& other) = delete;
			TransportContextV1& operator=(const TransportContextV1& other) = delete;

			const Network::ConnectionInfo& conninfo() const noexcept;
			Network::TCP::Connection& getconn() noexcept;
			const ContentEncodings& getEnconding() const noexcept;

			std::optional<HTTP::Request> nextRequest();
			void respond(const HTTP::Response& response);
			void reset() noexcept;
			bool hasPartialData() const noexcept;

			TransportFlags flags;
	};

	std::vector<uint8_t> compressContent(const std::vector<uint8_t>& data, ContentEncodings encoding);
};

#endif
