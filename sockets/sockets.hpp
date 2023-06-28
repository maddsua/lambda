#ifndef __LAMBDA_SOCKETS__
#define __LAMBDA_SOCKETS__

#include "../http/http.hpp"

#ifndef SOCKET
	typedef uint64_t SOCKET;
#endif

namespace Lambda::Socket {

	static const size_t network_chunksize_header = 2048;
	static const size_t network_chunksize_body = 131072;

	enum SockStatsuses {
		LAMBDASOCK_OK = 1,
		LAMBDASOCK_UNDEFINED = 0,
		LAMBDASOCK_INIT = -1,
		LAMBDASOCK_ADDRESS = -2,
		LAMBDASOCK_CREATE = -3,
		LAMBDASOCK_BIND = -4,
		LAMBDASOCK_LISTEN = -5,
		LAMBDASOCK_SHUTDOWN = -6,
		LAMBDASOCK_CLOSESOCK = -7,
		LAMBDASOCK_SEND = -8,
		LAMBDASOCK_SETOPT = -9,
		LAMBDASOCK_ACCEPT = -10
	};

	struct SocketStatusStruct {
		int64_t code = LAMBDASOCK_UNDEFINED;
		int64_t error = LAMBDASOCK_UNDEFINED;
	};

	class HTTPClientSocket {
		private:
			SOCKET hSocket = LAMBDASOCK_UNDEFINED;
			SocketStatusStruct _status;
			std::string _clientIPv4;

		public:
			HTTPClientSocket(SOCKET hParentSocket, time_t timeoutMs);
			HTTPClientSocket(SOCKET hParentSocket) : HTTPClientSocket(hParentSocket, 15000) {};
			~HTTPClientSocket();

			bool ok();
			SocketStatusStruct status();
			std::string ip();

			Lambda::HTTP::Request receiveMessage();
			bool sendMessage(Lambda::HTTP::Response& response);
	};

	class HTTPListenSocket {
		private:
			SOCKET hSocket = LAMBDASOCK_UNDEFINED;
			SocketStatusStruct _status;

		public:
			HTTPListenSocket() : HTTPListenSocket("8080") {};
			HTTPListenSocket(const char* listenPort);
			~HTTPListenSocket();

			bool ok();
			SocketStatusStruct status();

			HTTPClientSocket acceptConnection();
	};

};

#endif
