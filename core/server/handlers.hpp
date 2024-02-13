#ifndef __LIB_MADDSUA_LAMBDA_CORE_SERVER_HANDLERS__
#define __LIB_MADDSUA_LAMBDA_CORE_SERVER_HANDLERS__

#include "./options.hpp"
#include "../http/http.hpp"
#include "../websocket/websocket.hpp"
#include "../sse/sse.hpp"

#include <functional>

namespace Lambda {

	struct RequestContext {
		const std::string& contextID;
		const std::string& requestID;
		const Network::ConnectionInfo& conninfo;

		const std::function<SSE::Writer()>& startEventStream;
		const std::function<Websocket::WebsocketContext()>& upgrateToWebsocket;
	};

	struct HandlerResponse {
		std::optional<HTTP::Response> response;

		HandlerResponse() = default;
		HandlerResponse(const HTTP::Response& init);
		HandlerResponse(const std::string& init);
		HandlerResponse(const char* init);
		HandlerResponse(const std::vector<uint8_t>& init);
	};

	typedef std::function<HandlerResponse(const HTTP::Request&, const RequestContext&)> RequestCallback;
};

#endif
