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

	enum SockStatsuses {
		HSOCKERR_OK = 1,
		HSOCKERR_UNINITIALIZED = 0,
		HSOCKERR_INIT = -1,
		HSOCKERR_ADDRESS = -2,
		HSOCKERR_CREATE = -3,
		HSOCKERR_BIND = -4,
		HSOCKERR_LISTEN = -5,
		HSOCKERR_SHUTDOWN = -6,
		HSOCKERR_CLOSESOCK = -7,
		HSOCKERR_SEND = -8,
		HSOCKERR_SETOPT = 10,
		HSOCKERR_ACCEPT = 11
	};

	struct OpStatus {
		int64_t status = HSOCKERR_OK;
		int64_t code = HSOCKERR_OK;
	};

	class ClientSocket {
		private:
			SOCKET hSocket = HSOCKERR_UNINITIALIZED;
			int32_t socketStat = HSOCKERR_UNINITIALIZED;
			int32_t socketError = HSOCKERR_UNINITIALIZED;
			std::string clientIP;
			std::string clientIPv6;

		public:
			ClientSocket(SOCKET hParentSocket, time_t timeoutMs);
			ClientSocket(SOCKET hParentSocket) : ClientSocket(hParentSocket, 15000) {};
			~ClientSocket();

			bool ok();
			OpStatus status();

			HTTP::Request receiveMessage();
			OpStatus sendMessage(HTTP::Response& response);
	};

	class ListenSocket {
		private:
			SOCKET hSocket = HSOCKERR_UNINITIALIZED;
			int32_t socketStat = HSOCKERR_UNINITIALIZED;
			int32_t socketError = HSOCKERR_UNINITIALIZED;
			void _contructor(const char* listenPort);

		public:
			ListenSocket() : ListenSocket("8080") {};
			ListenSocket(const char* listenPort);
			~ListenSocket();

			bool ok();
			OpStatus status();

			ClientSocket acceptConnection();
	};

};

#endif
