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

	HTTP::Request receiveMessage(SOCKET clientSocket);
	bool sendMessage(SOCKET clientSocket, HTTP::Response& response);

	class Server {
		private:
			SOCKET ListenSocket;
			std::thread watchdogThread;
			void connectionWatchdog();
			void connectionHandler();
			bool handlerDispatched;
			bool running;
			
		public:
			Server();
			~Server();
	};

};

#endif
