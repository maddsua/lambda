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
		Net::ConnectionTimeouts connectionTimeouts;
		/**
		 * Max number of simultaneous connections
		 * 
		 * 0 = unlimited, but with the current joinlist implementation
		 * can cause slowdowns when a lot of connections are created at the same time
		 * 
		 * Note that 50 is a minimal allowed value and this option doesn't work at all
		 * if "useThreadList" is not enabled, and by default it is not.
		*/
		uint32_t maxConnections = 500;
		static const uint32_t minConnections = 50;

		/**
		 * Controls whether thread join list is in use.
		 * It's totally useless if you are not planning to destroy an instance,
		 * but can safe your ass from getting invalid references if you do.
		 * 
		 * In other words, if you only create one lambda instance per program
		 * and keep it until program exits - you don't need this.
		 * 
		 * The reason this option exists at all is that enabling it
		 * will tank the performance and in most projects you don't
		 * even need it in the first place.
		*/
		bool useThreadList = false;
	};

	struct ServerConfig : ServeOptions {
		/**
		 * Network options
		*/
		ServiceOptions service;
	};
};

#endif
