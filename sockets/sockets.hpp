#ifndef __LAMBDA_SOCKETS__
#define __LAMBDA_SOCKETS__

#include "../http/http.hpp"
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

namespace Lambda::Socket {

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

	class HTTPClientSocket {
		private:
			SOCKET hSocket = HSOCKERR_UNINITIALIZED;
			int32_t socketStat = HSOCKERR_UNINITIALIZED;
			int32_t socketError = HSOCKERR_UNINITIALIZED;
			std::string _clientIPv4;

		public:
			HTTPClientSocket(SOCKET hParentSocket, time_t timeoutMs);
			HTTPClientSocket(SOCKET hParentSocket) : HTTPClientSocket(hParentSocket, 15000) {};
			~HTTPClientSocket();

			bool ok();
			OpStatus status();
			std::string ip();

			Lambda::HTTP::Request receiveMessage();
			OpStatus sendMessage(Lambda::HTTP::Response& response);
	};

	class HTTPListenSocket {
		private:
			SOCKET hSocket = HSOCKERR_UNINITIALIZED;
			int32_t socketStat = HSOCKERR_UNINITIALIZED;
			int32_t socketError = HSOCKERR_UNINITIALIZED;

		public:
			HTTPListenSocket() : HTTPListenSocket("8080") {};
			HTTPListenSocket(const char* listenPort);
			~HTTPListenSocket();

			bool ok();
			OpStatus status();

			HTTPClientSocket acceptConnection();
	};

};

#endif