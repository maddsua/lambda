/*
	maddsua's lambda
	A C++ HTTP server framework
	2023 https://github.com/maddsua/lambda

	Header: The lambda itself
	
	Required libs:
		libwinmm
		libws2_32
		libbrotlicommon
		libbrotlidec
		libbrotlienc
		libz
*/


#ifndef H_MADDSUA_LAMBDA
#define H_MADDSUA_LAMBDA

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
			std::string clientIP;

			void* wormhole;		//	A poiential foot-shooter, be extremely careful when using it

			std::string method;
			std::string path;
			std::vector <datapair> searchQuery;
			std::vector <datapair> headers;
			std::string body;
		};
		struct lambdaResponse {
			unsigned int statusCode;
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

				//	Create a lambda server
				lambda() {
					memset(&wsaData, 0, sizeof(wsaData));
					ListenSocket = INVALID_SOCKET;
					handlerDispatched = true;
					running = false;
					instanceWormhole = nullptr;
				}
				//	lambda server destructor
				~lambda() {
					stop();
				}

				//	disable class copying
				lambda(const lambda&) = delete;
				void operator=(const lambda&) = delete;

				/**
				 * Start the lambda server
				 * @param port for lambda to listen to
				 * @param lambda handler function
				 * @param cfg server config (optional)
				*/
				actionResult start(const int port, std::function <lambdaResponse(lambdaEvent)> lambdaCallbackFunction);

				//	Stop this lambda server instance
				void stop();

				//	Apply configuration for this lambda server instance
				void setConfig(lambdaConfig config);

				/**
				 * Set a pointer to user-defined data or object
				 * @warning Don't mess up the type casting or else...
				*/
				void openWormhole(void* object);

				//	Null the user-defined data pointer
				void closeWormhole();

				//	Ture, if there are log entries available
				inline bool logsAvail() { return instanceLog.size(); }

				//	Get latest log entries
				std::vector <std::string> showLogs();

			private:

				WSADATA wsaData;
				SOCKET ListenSocket;

				bool running;
				std::thread worker;
				void connectDispatch();

				std::function <lambdaResponse(lambdaEvent)> requestCallback;
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
