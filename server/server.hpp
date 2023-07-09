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
			void addLogRecord(std::string message, int level);
			void addLogRecord(std::string message) { addLogRecord(message, LAMBDA_LOG); };

			void (*requestCallback)(Network::HTTPServer&, Context&) = nullptr;
			HTTP::Response (*requestCallbackServerless)(HTTP::Request&, Context&) = nullptr;

			void* instancePasstrough = nullptr;
			
		public:
			Server() { init(); };
			Server(uint16_t port) : openOnPort(port) { init(); };
			~Server();

			std::vector<LogEntry> logs();
			std::vector<std::string> logsText();
			bool hasNewLogs() { return this->logQueue.size() > 0; };

			void setServerCallback(void (*callback)(Network::HTTPServer&, Context&));
			void removeServerCallback();
			void setServerlessCallback(HTTP::Response (*callback)(HTTP::Request&, Context&));
			void removeServerlessCallback();

			void enablePasstrough(void* object);
			void disablePasstrough();

			bool isAlive() { return this->running; };

			Flags flags;
	};
	
}

#endif
