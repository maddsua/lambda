#ifndef __LAMBDA_SOCKETS__
#define __LAMBDA_SOCKETS__

#include "../http/http.hpp"

#ifdef _WIN32

	//#pragma comment(lib,"ws2_32.lib")
	//#define WIN32_LEAN_AND_MEAN
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

	HTTP::Request receive(SOCKET* client);
	bool transmit(SOCKET* client, HTTP::Response& response);

};

#endif
