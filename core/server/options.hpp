#ifndef __LIB_MADDSUA_LAMBDA_CORE_SERVER_OPTIONS__
#define __LIB_MADDSUA_LAMBDA_CORE_SERVER_OPTIONS__

#include "../http/transport.hpp"
#include "../network/network.hpp"

namespace Lambda {

	/**
	 * Server log options
	*/
	struct LogOptions {
		/**
		 * Log events like connections and transport creations/destructions
		*/
		bool transportEvents = false;
		/**
		 * Log http requests
		*/
		bool requests = true;
		/**
		 * Display server start message
		*/
		bool startMessage = true;
	};

	/**
	 * What type of error page to use
	*/
	enum struct ErrorResponseType {
		/**
		 * Return a plaintext error message
		*/
		Plain,
		/**
		 * Display a nice html page with an error message
		*/
		HTML,
		/**
		 * Return error message as a JSON object
		*/
		JSON
	};

	/**
	 * Web server options
	*/
	struct ServeOptions {
		/**
		 * Logging options
		*/
		LogOptions loglevel;
		/**
		 * HTTP transport options
		*/
		HTTP::Transport::TransportOptions transport;
		/**
		 * Choose error page type
		*/
		ErrorResponseType errorResponseType = ErrorResponseType::HTML;
	};

	/**
	 * Service settings (sockets stuff and such)
	*/
	struct ServiceOptions {
		/**
		 * Service port
		*/
		uint16_t port = 8180;
		/**
		 * Allow fast port reuse
		*/
		bool fastPortReuse = false;
		/**
		 * Set default connection timeout
		*/
		Network::ConnectionTimeouts connectionTimeouts;
		/**
		 * Max number of simultaneous connections
		 * 
		 * 0 = unlimited, but with the current joinlist implementation
		 * can cause slowdowns when a lot of connections are created at the same time
		 * 
		 * Note that 50 is a minimal allowed value
		*/
		uint32_t maxConnections = 500;
		static const uint32_t minConnections = 50;
	};

	struct ServerConfig : ServeOptions {
		/**
		 * Network options
		*/
		ServiceOptions service;
	};
};

#endif
