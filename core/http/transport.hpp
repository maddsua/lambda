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

	/**
	 * Protocol error indicated an error within HTTP protocol but it may not be linked to any network errors
	 * and since that we may want to respond to it with an error page.
	 * 
	 * An example protocol error would be a request payload that is too large or a malformed request header.
	*/
	class ProtocolError : public Lambda::Error {
		public:
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
			HTTP::Request* m_next = nullptr;

		public:
			TransportContextV1(Network::TCP::Connection& connInit, const TransportOptions& optsInit);

			TransportContextV1(const TransportContextV1& other) = delete;
			TransportContextV1& operator=(const TransportContextV1& other) = delete;

			Network::TCP::Connection& tcpconn() const noexcept;
			const Network::ConnectionInfo& conninfo() const noexcept;
			const TransportOptions& options() const noexcept;
			const ContentEncodings& getEnconding() const noexcept;
			bool ok() const noexcept;

			bool awaitNext();
			HTTP::Request nextRequest();
			void respond(const HTTP::Response& response);
			void reset() noexcept;
			bool hasPartialData() const noexcept;

			TransportFlags flags;
	};
};

#endif
