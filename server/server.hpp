#ifndef __LAMBDA_SERVER__
#define __LAMBDA_SERVER__

#include "../network/network.hpp"
#include <deque>

namespace Lambda {

	struct LogEntry {
		std::string message;
		std::string datetime;
		time_t timestamp = 0;
		int loglevel;
	};

	enum LogLevel {
		LAMBDA_LOG = 0,
		LAMBDA_LOG_ERROR = 1,
		LAMBDA_LOG_WARN = 2,
		LAMBDA_LOG_INFO = 3,
		LAMBDA_LOG_CRITICAL = 4,
	};

	struct Context {
		std::string clientIP;
		//	A potential foot-shooter, be extremely careful when using it
		void* passtrough;
	};

	struct Flags {
		bool compressionUseGzip = false;
		bool compressionUseBrotli = false;
	};

	/**
	 * Probably the thing you are here
	*/
	class Server {
		private:
			Lambda::Network::ListenSocket* ListenSocketObj;
			std::thread* watchdogThread;
			bool handlerDispatched;
			bool running;
			uint16_t openOnPort = 8080;
			void init();

			void connectionWatchdog();
			void connectionHandler();

			std::mutex mtLock;
			std::deque<LogEntry> logQueue;
			void addLogRecord(std::string message, LogLevel level);
			void addLogRecord(std::string message) { addLogRecord(message, LAMBDA_LOG); };

			void (*requestCallback)(Network::HTTPServer&, Context&) = nullptr;
			HTTP::Response (*requestCallbackServerless)(HTTP::Request&, Context&) = nullptr;

			void* instancePasstrough = nullptr;
			
		public:

			/**
			 * Lambda server class
			*/
			Server() { init(); };
			Server(uint16_t port) : openOnPort(port) { init(); };
			~Server();

			/**
			 * Get new log records.
			 * 
			 * After calling this function the log deque will be purged, so subsequential calls to this function will return nothing until new records will appear
			*/
			std::vector<LogEntry> logs();

			/**
			 * Get new logs in a form of string vector (text array for the js and snake-devs)
			 * 
			 * After calling this function the log deque will be purged, so subsequential calls to this function will return nothing until new records will appear
			*/
			std::vector<std::string> logsText();

			/**
			 * true, if there are unread logs
			*/
			bool hasNewLogs() { return this->logQueue.size() > 0; };

			/**
			 * Server callback reseives a client connection to which it can directly send or receive data.
			 * 
			 * Use this to setup websocket connection or something even more overcomplicated
			*/
			void setServerCallback(void (*callback)(Network::HTTPServer&, Context&));
			void removeServerCallback();

			/**
			 * Serverless callback received a Response structure, it can do whatever but mush return a Response object. This is good for serving static files or most of the web APIs.
			 * 
			 * Use server callback (setServerCallback) if you need realtime or two way communication.
			*/
			void setServerlessCallback(HTTP::Response (*callback)(HTTP::Request&, Context&));
			void removeServerlessCallback();

			/**
			 * Set a custom struct pointer that can be accessed in the request context of all handler functions.
			 * 
			 * Or you can use global variables, but this is way cooler anyway.
			 * 
			 * Pro-tip: Check for null when dereferencing
			*/
			void enablePasstrough(void* object);

			/**
			 * Remove passtrough pointer (will be set to null)
			*/
			void disablePasstrough();

			/**
			 * True, is server is not toasted
			*/
			bool isAlive() { return this->running; };

			/**
			 * Some configuration clags, can be changed on the fly
			*/
			Flags flags;
	};

	/**
	 * Styled status/error page generator
	*/
	HTTP::Response serviceResponse(uint16_t httpStatus, const std::string& text);

}

#endif
