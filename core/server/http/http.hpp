#ifndef __LIB_MADDSUA_LAMBDA_CORE_HTTPSERVER__
#define __LIB_MADDSUA_LAMBDA_CORE_HTTPSERVER__

#include "../server.hpp"
#include "../../network/tcp/connection.hpp"

#include <optional>

namespace Lambda::HTTPServer {

	struct HTTPReaderContext {
		Network::TCP::Connection& conn;
		const HTTPTransportOptions& options;
		const Network::ConnectionInfo& conninfo;
		std::vector<uint8_t> buffer;
		bool keepAlive = false;
	};

	enum struct ContentEncodings {
		None, Brotli, Gzip, Deflate,
	};

	struct HTTPTransportContext {
		ContentEncodings acceptsEncoding = ContentEncodings::None;
		bool keepAlive = false;
	};

	struct IncomingRequest : HTTPTransportContext {
		HTTP::Request request;
	};

	struct HTTPWriterContext : HTTPTransportContext {
		Network::TCP::Connection& conn;
	};

	struct ConnectionContext : HTTPReaderContext {
		ContentEncodings acceptsEncoding = ContentEncodings::None;
	};

	struct IncomingConnection {
		private:

			enum struct ActiveProtocol {
				HTTP, WS
			};

			HTTPServer::ConnectionContext ctx;
			ActiveProtocol activeProto = ActiveProtocol::HTTP;

		public:
			IncomingConnection(Network::TCP::Connection& conn, const HTTPTransportOptions& opts);

			IncomingConnection(const IncomingConnection& other) = delete;
			IncomingConnection& operator=(const IncomingConnection& other) = delete;

			std::optional<HTTP::Request> nextRequest();
			void respond(const HTTP::Response& response);

			WSServer::WebsocketContext upgrateToWebsocket();
	};
};

#endif
