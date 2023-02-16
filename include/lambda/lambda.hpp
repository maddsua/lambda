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

	#include <winsock2.h>
	#include <ws2tcpip.h>
	#include <stdint.h>
	#include <time.h>

	#include <string>
	#include <vector>
	#include <thread>
	#include <functional>

	#include "httpcore.hpp"
	#include "localdb.hpp"
	#include "crypto.hpp"
	
	#include "../maddsua/base64.hpp"


	#define LAMBDA_HTTP_ATTEMPTS	(3)
	#define LAMBDAHTTP_HEADER_CHUNK		(2048)
	#define LAMBDAHTTP_BODY_CHUNK		(131072)
	#define LAMBDA_HTTP_USERAGENT	"maddsua/lambda"
	#define LAMBDA_HTTP_ACCEPTENC	"gzip, deflate"

	#define LAMBDA_LOG_INFO			(1)
	#define LAMBDA_LOG_WARN			(0)
	#define LAMBDA_LOG_ERR			(-1)

	#define LAMBDA_REQ_LAMBDA		(1)
	#define LAMBDA_REQ_WEBSOCK		(2)

	#define LAMBDA_DSP_SLEEP		(10)

	#define LAMBDA_FS_READ_CHUNK	(1048576)


	namespace lambda {

		//	Check if WSA (Windows Sockets API) is ready to use
		bool socketsReady();

		//	Receive http data from a socket in one go
		httpRequest socketGetHTTP(SOCKET* client);

		//	Send http data to a socket in one go
		actionResult socketSendHTTP(SOCKET* client, std::string startline, httpHeaders& headers, const std::string& body);

		struct fetchResult {
			std::string errors;
			unsigned int statusCode;
			std::string statusText;
			httpHeaders headers;
			std::string body;
		};
		
		struct fetchData {
			std::string method;
			std::vector <stringPair> headers;
			std::string body;
		};

		//	The same as javascript's fetch(). Sends request to a URL and returns server response
		fetchResult fetch(std::string url);
		fetchResult fetch(std::string url, const fetchData& data);

		struct Event {
			std::string httpversion;
			std::string requestID;
			std::string clientIP;

			void* wormhole;		//	A poiential foot-shooter, be extremely careful when using it

			std::string method;
			jstring path;
			httpHeaders headers;
			httpSearchQuery searchQuery;
			std::string body;
		};
		
		struct Response {
			unsigned int statusCode;
			//	I'm not constructing headers object directry,
			//	so you won't have to add an extra pair
			//	of squiggly brackets every time you return some headers 
			std::vector <stringPair> headers;
			std::string body;
		};

		/**
		 * @param compression_enabled enable content compression
		 * @param compression_allFileTypes compress all file types
		 * @param compression_preferBr prefer brotli compression, if client accepts
		 * @param mutlipeInstances check is WSA is enabled on instance init and don't disable WSA after the instance shutdown
		*/
		struct Config {
			bool compression_enabled = true;
			bool compression_allFileTypes = false;
			bool compression_preferBr = false;
			bool mutlipeInstances = false;
		};

		struct Context {
			std::string requestId;
			std::string clientIP;
			time_t started = 0;
			int requestType = 0;
		};

		struct LogEntry {
			std::string requestId;
			std::string clientIP;
			std::string message;
			time_t timestamp = 0;
			int type = 0;
		};

		class lambda {
			public:
				//	constructor/destructor
				lambda();
				~lambda();
				//	disable class copying
				lambda(const lambda&) = delete;
				void operator=(const lambda&) = delete;
				//	Start this lambda server instance
				actionResult start(const int port, std::function <Response(Event)> lambdaCallbackFunction);
				//	Stop this lambda server instance
				void stop();
				//	Apply configuration for this lambda server instance
				void setConfig(Config config);
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

				std::function <Response(Event)> requestCallback;
				bool handlerDispatched;
				void handler();

				void addLogEntry(Context context, int type, std::string message);
				std::vector <LogEntry> instanceLog;

				std::mutex threadLock;

				Config instanceConfig;
				std::string serverTime(time_t timestamp);
				std::string serverTime();

				void* instanceWormhole;
		};
	}

#endif
