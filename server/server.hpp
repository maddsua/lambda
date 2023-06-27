#ifndef __LAMBDA_SERVER__
#define __LAMBDA_SERVER__

#include "../sockets/sockets.hpp"
#include <deque>
#include <mutex>
#include <functional>

namespace Lambda {

	struct LogEntry {
		std::string message;
		std::string datetime;
		time_t timestamp = 0;
	};

	struct Context {
		std::string clientIP;
	};

	class Server {
		private:
			LambdaSocket::HTTPListenSocket* ListenSocketObj;
			std::thread* watchdogThread;
			bool handlerDispatched;
			bool running;

			void connectionWatchdog();
			void connectionHandler();

			std::mutex mtLock;
			std::deque<LogEntry> logQueue;
			void addLogRecord(std::string message);

			//std::function <void(HTTP::Request, Context)> requestCallback;
			//std::function <HTTP::Response(HTTP::Request, Context)> requestCallbackServerless;
			void (*requestCallback)(HTTP::Request, Context) = nullptr;
			HTTP::Response (*requestCallbackServerless)(HTTP::Request, Context) = nullptr;
			
		public:
			Server();
			~Server();
			std::vector<LogEntry> logs();
			std::vector<std::string> logsText();
			bool hasNewLogs();

			void setServerCallback(void (*callback)(HTTP::Request, Context));
			void removeServerCallback();
			void setServerlessCallback(HTTP::Response (*callback)(HTTP::Request, Context));
			void removeServerlessCallback();
	};
	
}

#endif
