#ifndef __LIB_MADDSUA_LAMBDA_CORE_HTTP_TRANSPORT__
#define __LIB_MADDSUA_LAMBDA_CORE_HTTP_TRANSPORT__

#include "./http.hpp"
#include "../network/tcp/connection.hpp"
#include "../crypto/crypto.hpp"
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
	 * ProtocolError when request cannot be processed but it's not caused by a newtwork error or a dropped connection.
	 * 
	 * It could be a request payload that is too large or a malformed request header.
	 * 
	 * Note: to avoid confusion, protocol error should be thrown before pushing a request to the transport queue (or whatever equivalent there is)
	*/
	class ProtocolError : public Lambda::Error {
		public:
			const std::optional<HTTP::Status> respondStatus;

			ProtocolError(const std::string& message) : Error(message) {}
			ProtocolError(const std::string& message, HTTP::Status respondWithStatus) : Error(message), respondStatus(respondWithStatus) {}
	};

	struct IncomingRequest {
		const HTTP::Request request;
		const Crypto::ShortID id;
	};

	struct ResponseContext {
		HTTP::Response response;
		const Crypto::ShortID& id;
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

			virtual const std::string& contextID() const noexcept = 0;

			virtual Network::TCP::Connection& tcpconn() const noexcept = 0;
			virtual const Network::ConnectionInfo& conninfo() const noexcept = 0;
			virtual const TransportOptions& options() const noexcept = 0;
			virtual const ContentEncodings& getEnconding() const noexcept = 0;
			virtual bool isConnected() const noexcept = 0;

			virtual bool awaitNext() = 0;
			virtual IncomingRequest nextRequest() = 0;
			virtual void respond(const ResponseContext& responsectx) = 0;

			virtual std::vector<uint8_t> readRaw() = 0;
			virtual std::vector<uint8_t> readRaw(size_t expectedSize) = 0;
			virtual void writeRaw(const std::vector<uint8_t>& data) = 0;

			virtual void reset() noexcept = 0;
			virtual bool hasPartialData() const noexcept = 0;
			virtual void close() = 0;

			TransportFlags flags;
	};
};

#endif
