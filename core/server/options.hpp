#ifndef __LIB_MADDSUA_LAMBDA_CORE_SERVER_OPTIONS__
#define __LIB_MADDSUA_LAMBDA_CORE_SERVER_OPTIONS__

#include <cstdint>

namespace Lambda {

	struct LogOptions {
		bool connections = false;
		bool requests = false;
		bool timestamps = false;
	};

	struct HTTPTransportOptions {
		bool useCompression = true;
		bool reuseConnections = true;
	};

	enum struct ErrorResponseType {
		HTML, JSON
	};

	struct ServeOptions {
		LogOptions loglevel;
		HTTPTransportOptions transport;
		ErrorResponseType errorResponseType = ErrorResponseType::HTML;
	};

	struct ServiceOptions {
		uint16_t port = 8180;
		bool fastPortReuse = false;
		uint32_t connectionTimeout = 15000;
	};

	struct ServerConfig : ServeOptions {
		ServiceOptions service;
	};
};

#endif
