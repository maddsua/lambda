#ifndef __LAMBDA_SERVER__
#define __LAMBDA_SERVER__

#include "../sockets/sockets.hpp"
#include <deque>
#include <mutex>

namespace Lambda {

	struct LogEntry {
		std::string message;
		std::string datetime;
		time_t timestamp = 0;
	};

	class Server {
		private:
			HTTPSocket::ListenSocket* ListenSocketObj;
			std::thread* watchdogThread;
			bool handlerDispatched;
			bool running;

			void connectionWatchdog();
			void connectionHandler();

			std::mutex mtLock;
			std::deque<LogEntry> logQueue;
			void addLogRecord(std::string message);

			//ServerStat ();
			
		public:
			Server();
			~Server();
			std::vector<LogEntry> logs();
			std::vector<std::string> logsText();
			bool hasNewLogs();
	};
	
}

#endif
