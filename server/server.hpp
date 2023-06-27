#ifndef __LAMBDA_SERVER__
#define __LAMBDA_SERVER__

#include "../sockets/sockets.hpp"

namespace Lambda {

	class Server {
		private:
			HTTPSocket::ListenSocket* ListenSocketObj;
			std::thread* watchdogThread;
			bool handlerDispatched;
			bool running;

			void connectionWatchdog();
			void connectionHandler();

			//ServerStat ();
			
		public:
			Server();
			~Server();
	};
	
}

#endif