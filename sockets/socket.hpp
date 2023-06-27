#ifndef __LAMBDA_SOCKETS__
#define __LAMBDA_SOCKETS__

#include "../http/http.hpp"
#include "../lambda/exception.hpp"
#include <thread>

#ifdef _WIN32

	//#pragma comment(lib,"ws2_32.lib")
	#define WIN32_LEAN_AND_MEAN
	//#undef TEXT
	#include <winsock2.h>
	#include <ws2tcpip.h>

#else

	typedef int SOCKET;
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netdb.h>
	#include <unistd.h>
	#include <arpa/inet.h>
	static const int INVALID_SOCKET = -1;
	static const int SOCKET_ERROR   = -1;

#endif

namespace HTTPSocket {

	static const size_t network_chunksize_header = 2048;
	static const size_t network_chunksize_body = 131072;

	enum Error {
		HSOCKERR_OK = 0,
		HSOCKERR_INIT = -1,
		HSOCKERR_LHADDRESS = -2,
		HSOCKERR_CREATE = -3,
		HSOCKERR_BIND = -4,
		HSOCKERR_LISTEN = -5,
		HSOCKERR_SHUTDOWN = -6,
		HSOCKERR_CLOSESOCK = -7,
		HSOCKERR_SEND = -8,
		HSOCKERR_SENDOPT = 10
	};

	struct OpStatus {
		Error error = HSOCKERR_OK;
		uint32_t code = HSOCKERR_OK;
	};

	HTTP::Request receiveMessage(SOCKET sockref);
	OpStatus sendMessage(SOCKET sockref, HTTP::Response& response);
	OpStatus createAndListen(SOCKET* sockPtr, const char* port);
	OpStatus disconnect(SOCKET sockref);
	OpStatus setConnectionTimeout(SOCKET sockref, uint32_t sockTimeoutMs);

	class Server {
		private:
			SOCKET ListenSocket;
			std::thread watchdogThread;
			bool handlerDispatched;
			bool running;

			void connectionWatchdog();
			void connectionHandler();

			//ServerStat ();
			
		public:
			Server();
			~Server();
	};

};

#endif
