#ifndef __LIB_MADDSUA_LAMBDA_CORE_SERVER_OPTIONS__
#define __LIB_MADDSUA_LAMBDA_CORE_SERVER_OPTIONS__

#include "../http/transport.hpp"
#include "../network/network.hpp"

namespace Lambda {

	struct LogOptions {
		bool transportEvents = false;
		bool requests = true;
		bool startMessage = true;
	};

	enum struct ErrorResponseType {
		HTML, JSON
	};

	struct ServeOptions {
		LogOptions loglevel;
		HTTP::Transport::TransportOptions transport;
		ErrorResponseType errorResponseType = ErrorResponseType::HTML;
	};

	struct ServiceOptions {
		uint16_t port = 8180;
		bool fastPortReuse = false;
		Network::ConnectionTimeouts connectionTimeouts;
		uint32_t maxConnections = 0;
	};

	struct ServerConfig : ServeOptions {
		ServiceOptions service;
	};
};

#endif
