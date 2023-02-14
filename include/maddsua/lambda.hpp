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

#include <mutex>

#include "http.hpp"
#include "crypto.hpp"
#include "base64.hpp"
#include "fs.hpp"
#include "radishdb.hpp"

#define LAMBDALOG_INFO		(1)
#define LAMBDALOG_WARN		(0)
#define LAMBDALOG_ERR		(-1)

#define LAMBDAREQ_LAMBDA	(1)
#define LAMBDAREQ_WEBSOCK	(2)

#define LAMBDA_DSP_SLEEP	(10)

namespace lambda {

	struct lambdaEvent {
		std::string httpversion;
		std::string requestID;

		//	A poiential foot-shooter, be extremely careful when using it
		void* wormhole;

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

	struct lambdaInvokContext {
		std::array <uint8_t, UUID_BYTES> uuid;
		time_t started = 0;
		std::string clientIP;
		short requestType = 0;
	};

	struct lambdaLogEntry {
		time_t timestamp;
		std::array <uint8_t, UUID_BYTES> requestId;
		std::string clientIP;
		std::string message;
		short type;
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
				stop();
			}


			/**
			 * Start the lambda server
			 * @param port for lambda to listen to
			 * @param lambda handler function
			 * @param cfg server config (optional)
			*/
			actionResult start(const int port, std::function <lambdaResponse(lambdaEvent)> lambda);

			void setConfig(lambdaConfig config);

			void openWormhole(void* object);
			void closeWormhole();

			/**
			 * Stop this lambda server
			*/
			void stop();

			/**
			 * Ture, if there are log entries available
			*/
			inline bool logsAvail() { return instanceLog.size(); }

			/**
			 * Get last log entries
			*/
			std::vector <std::string> showLogs();

		private:
			WSADATA wsaData;
			SOCKET ListenSocket;

			bool running;
			std::thread worker;
			void connectDispatch();

			std::function<lambdaResponse(lambdaEvent)> callback;
			bool handlerDispatched;
			void handler();

			void addLogEntry(lambdaInvokContext context, short type, std::string message);
			std::vector <lambdaLogEntry> instanceLog;

			std::mutex threadLock;

			lambdaConfig instanceConfig;
			std::string serverTime(time_t timestamp);
			std::string serverTime();

			void* instanceWormhole;
	};

}

#endif
