/*
	Required libs:
		libwinmm
		libws2_32
		libbrotlicommon
		libbrotlidec
		libbrotlienc
		libz
*/


#ifndef _maddsua_http_lambda
#define _maddsua_http_lambda

//#include <mutex>

#include "http.hpp"
#include "crypto.hpp"
#include "hex.hpp"
#include "base64.hpp"

#define LAMBDALOG_INFO	(1)
#define LAMBDALOG_WARN	(0)
#define LAMBDALOG_ERR	(-1)

namespace lambda {

	struct lambdaEvent {
		std::string httpversion;
		std::string requestID;

		std::string method;
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

	struct lambdaRequestContext {
		std::string uid;
		size_t activeThreads;
		time_t started;
		bool signalDone = false;
		bool signalStop = false;
	};

	struct lambdaLogEntry {
		short type;
		std::string requestId;
		std::string message;
		time_t timestamp;
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
			actionResult init(const uint32_t port, std::function <lambdaResponse(lambdaEvent)> lambda);
			inline actionResult init(const uint32_t port, std::function <lambdaResponse(lambdaEvent)> lambda, lambdaConfig cfg) {
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
			std::vector <std::string> showLogs();

		private:
			WSADATA wsaData;
			SOCKET ListenSocket;

			bool running;
			std::thread worker;
			void connectManager();

			std::function<lambdaResponse(lambdaEvent)> callback;
			bool handlerDispatched;
			void handler(lambdaRequestContext& context);

			void addLogEntry(std::string requestID, short type, std::string message);
			std::vector <lambdaLogEntry> serverlog;

			std::vector <lambdaRequestContext> activeThreads;
			//std::mutex threadLock;

			lambdaConfig config;
	};

	namespace fs {
		bool writeBinary(const std::string path, const std::string* data);
		bool readBinary(const std::string path, std::string* dest);
	}

}


#endif