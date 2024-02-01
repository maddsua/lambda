#ifndef __LIB_MADDSUA_LAMBDA_CORE_SERVER_HANDLERS__
#define __LIB_MADDSUA_LAMBDA_CORE_SERVER_HANDLERS__

#include "./server.hpp"
#include "./http.hpp"
#include "../network/tcp/connection.hpp"

#include <future>
#include <queue>
#include <optional>

namespace Lambda::Server::Handlers {

	struct ReaderContext {
		Network::TCP::Connection& conn;
		const HTTPTransportOptions& options;
		const Network::ConnectionInfo& conninfo;
		std::vector<uint8_t> buffer;
		bool keepAlive = false;
	};

	std::optional<HTTPServer::RequestQueueItem> requestReader(ReaderContext& ctx);
	void writeResponse(HTTP::Response& response, Network::TCP::Connection& conn, HTTPServer::ContentEncodings preferEncoding);

	void asyncRequestReader(Network::TCP::Connection& conn, const HTTPTransportOptions& options, HTTPServer::HttpRequestQueue& queue);

	void httpServerlessHandler(Network::TCP::Connection&& conn, const ServeOptions& config, const HTTPRequestCallback& handlerCallback) noexcept;
	void httpExtendedHandler(Network::TCP::Connection&& conn, const ServeOptions& config, const ConnectionCallback& handlerCallback) noexcept;

};

#endif
