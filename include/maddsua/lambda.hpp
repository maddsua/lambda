#ifndef _maddsua_http_lambda
#define _maddsua_http_lambda


#include "http.hpp"


namespace maddsua {

	struct lambdaEvent {
		std::string method;
		std::string httpversion;
		std::string path;
		std::vector <datapair> searchQuery;
		std::vector <datapair> headers;
		std::string body;
	};
	struct lambdaResponse {
		uint16_t statusCode;
		std::vector <datapair> headers;
		std::string body;
	};
	/**
	 * @param compression_enabled enable content compression
	 * @param compression_allFileTypes compress all file types
	 * @param compression_preferBr prefer brotli compression, if client accepts
	 * @param mutlipeInstances check is WSA is enabled on instance init and don't disable WSA after the instance shutdown
	*/
	struct lambdaConfig {
		bool compression_enabled = true;
		bool compression_allFileTypes = false;
		bool compression_preferBr = false;
		bool mutlipeInstances = false;
	};

	class lambda {

		public:
			/**
			 * Create a lambda server
			*/
			lambda() {
				wsaData = {0};
				ListenSocket = INVALID_SOCKET;
				handlerDispatched = true;
				running = false;
			}
			lambda(const lambda & obj) {
				//	a copy constructor so IntelliSense would shut the f up
			}
			lambda(lambda && obj) {
				//	a move constructor for the same purpose
				//	you can't actually use them due to std::threads not being copyable
				//	and copying ov moving the server does not make any sence to me, bc it's a "running function" and not a data object,
				//	so I can't think of scenario wnen you need to copy it
			}
			~lambda() {
				close();
			}


			/**
			 * Start the lambda server
			 * @param port for lambda to listen to
			 * @param lambda handler function
			 * @param cfg server config (optional)
			*/
			actionResult init(const char* port, std::function <lambdaResponse(lambdaEvent)> lambda);
			inline actionResult init(const char* port, std::function <lambdaResponse(lambdaEvent)> lambda, lambdaConfig cfg) {
				config = cfg;
				return init(port, lambda);
			}

			/**
			 * Stop this lambda server
			*/
			void close();

			/**
			 * Ture, if there are log entries available
			*/
			inline bool logsAvail() { return serverlog.size(); }

			/**
			 * Get last log entries
			*/
			inline std::vector <std::string> logs() {
				auto temp = serverlog;
				serverlog.erase(serverlog.begin(), serverlog.end());
				return temp;
			}

		private:
			WSADATA wsaData;
			SOCKET ListenSocket;

			bool running;
			std::thread worker;
			void connectManager();

			std::function<lambdaResponse(lambdaEvent)> callback;
			bool handlerDispatched;
			void handler();
			std::vector <std::string> serverlog;
			void addLogEntry(std::string type, std::string text);

			lambdaConfig config;

			const std::vector<std::string> compressableTypes = {
				"text",
				"application"
			};
	};

}

#endif