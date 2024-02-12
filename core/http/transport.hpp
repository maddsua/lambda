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

	class TransportContext {
		public:
			TransportContext() = default;
			TransportContext(TransportContext&& other) = delete;
			TransportContext(const TransportContext& other) = delete;
			virtual ~TransportContext() = default;

			TransportContext& operator=(const TransportContext& other) = delete;
			TransportContext& operator=(TransportContext&& other) = delete;

			virtual Network::TCP::Connection& tcpconn() const noexcept = 0;
			virtual const Network::ConnectionInfo& conninfo() const noexcept = 0;
			virtual const TransportOptions& options() const noexcept = 0;
			virtual const ContentEncodings& getEnconding() const noexcept = 0;
			virtual bool ok() const noexcept = 0;

			virtual bool awaitNext() = 0;
			virtual HTTP::Request nextRequest() = 0;
			virtual void respond(const HTTP::Response& response) = 0;
			virtual void reset() noexcept = 0;
			virtual bool hasPartialData() const noexcept = 0;

			TransportFlags flags;
	};
};

#endif
