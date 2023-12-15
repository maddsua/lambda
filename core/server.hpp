#ifndef __LIB_MADDSUA_LAMBDA_CORE_SERVER__
#define __LIB_MADDSUA_LAMBDA_CORE_SERVER__

#include <functional>
#include <future>
#include <optional>

#include "./http.hpp"
#include "./network.hpp"

namespace Lambda {

	namespace Server {

		struct LogItem {
			std::string value;

			LogItem(const std::string& thing);
			LogItem(const char* thing);
			LogItem(bool thing);
			LogItem(char thing);
			LogItem(unsigned char thing);
			LogItem(short thing);
			LogItem(unsigned short thing);
			LogItem(int thing);
			LogItem(unsigned int thing);
			LogItem(float thing);
			LogItem(double thing);
			LogItem(long thing);
			LogItem(unsigned long thing);
			LogItem(long long thing);
			LogItem(unsigned long long thing);
			LogItem(long double thing);
		};

		class Console {
			private:
				std::string id;
				std::string serializeEntries(const std::initializer_list<LogItem>& list) const;

			public:
				Console(const std::string& setid) : id(setid) {}

				void log(std::initializer_list<LogItem> list) const;
				void error(std::initializer_list<LogItem> list) const;
				void warn(std::initializer_list<LogItem> list) const;
		};

		struct ServeOptions {
			struct {
				bool logConnections = false;
				bool logRequests = true;			
			} loglevel;
			struct {
				bool httpCompressionEnabled = true;		
			} transport;
		};

		struct HttpHandlerOptions : ServeOptions {
			std::string contextID;
		};

		struct RequestContext {
			std::string requestID;
			Network::ConnInfo conninfo;
			Console console;
		};

		typedef std::function<HTTP::Response(const HTTP::Request& request, const RequestContext& context)> HttpHandlerFunction;
		void handleHTTPConnection(Network::TCPConnection&& conn, HttpHandlerFunction handler, const HttpHandlerOptions& options);

		HTTP::Response serviceResponse(int statusCode, std::optional<std::string> errorMessage);

		struct ServiceOptions {
			uint16_t port = 8180;
			bool fastPortReuse = false;
		};

		struct HttpServerConfig : ServeOptions {
			ServiceOptions service;
		};

		class HttpServer {
			private:
				Network::TCPListenSocket* listener = nullptr;
				Server::HttpHandlerFunction handler;
				HttpServerConfig config;
				std::thread watchdogWorker;
				bool terminated = false;

			public:
				HttpServer(Server::HttpHandlerFunction handlerFunction, HttpServerConfig init);
				~HttpServer();

				void softShutdownn();
				void immediateShutdownn();
				void awaitFinished();

				const HttpServerConfig& getConfig() const noexcept;
		};
	};

	typedef Server::HttpServer HttpServer;
	typedef Server::HttpServerConfig HttpServerConfig;

	typedef HTTP::Request Request;
	typedef Server::RequestContext Context;

	typedef Server::Console Console;
};

#endif
