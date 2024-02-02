#ifndef __LIB_MADDSUA_LAMBDA_CORE_HTTPSERVER_TRANSPORT__
#define __LIB_MADDSUA_LAMBDA_CORE_HTTPSERVER_TRANSPORT__

#include "./http.hpp"
#include "../server.hpp"
#include "../../network/tcp/connection.hpp"

#include <optional>

namespace Lambda::HTTPServer::Transport {

	std::optional<IncomingRequest> requestReader(HTTPReaderContext& ctx);
	void writeResponse(const HTTP::Response& response, const HTTPWriterContext& ctx);

	std::optional<std::pair<std::string, std::string>> parseBasicAuth(const std::string& header);

};

#endif
