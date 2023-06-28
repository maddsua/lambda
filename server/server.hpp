#ifndef __LAMBDA_SERVER__
#define __LAMBDA_SERVER__

#include "../sockets/sockets.hpp"
#include <deque>
#include <mutex>
#include <thread>

namespace Lambda {

	struct LogEntry {
		std::string message;
		std::string datetime;
		time_t timestamp = 0;
	};

	struct Context {
		std::string clientIP;
		void* passtrough;			//	A potential foot-shooter, be extremely careful when using it
	};

	enum ServerStatus {
		LAMBDA_UNDEFINED = 0,
		LAMBDA_OK = 1,
		LAMBDA_LISTENSOCKERR = -1,
		LAMBDA_LISTENSOCKRESTART = 2,
	};

	enum ServerLogLevel {
		LAMBDA_LOG = 0,
		LAMBDA_LOG_ERROR = 1,
		LAMBDA_LOG_WARN = 2,
		LAMBDA_LOG_INFO = 2,
	};

	struct SeverStatStruct {
		int64_t code = LAMBDA_UNDEFINED;
		int64_t error = LAMBDA_UNDEFINED;
		int64_t apierror = LAMBDA_UNDEFINED;
	};

	class Server {
		private:
			Lambda::Socket::HTTPListenSocket* ListenSocketObj;
			std::thread* watchdogThread;
			bool handlerDispatched;
			bool running;

			void connectionWatchdog();
			void connectionHandler();

			std::mutex mtLock;
			std::deque<LogEntry> logQueue;
			void addLogRecord(std::string message);

			void (*requestCallback)(HTTP::Request&, Context&) = nullptr;
			HTTP::Response (*requestCallbackServerless)(HTTP::Request&, Context&) = nullptr;

			void* instancePasstrough = nullptr;
			void setPasstrough(void* object);
			void removePasstrough();

			SeverStatStruct _status;
			
		public:
			Server();
			~Server();

			std::vector<LogEntry> logs();
			std::vector<std::string> logsText();
			bool hasNewLogs();

			void setServerCallback(void (*callback)(HTTP::Request&, Context&));
			void removeServerCallback();
			void setServerlessCallback(HTTP::Response (*callback)(HTTP::Request&, Context&));
			void removeServerlessCallback();

			int status();
			std::string statusText();
	};
	
}

#endif
