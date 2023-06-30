#ifndef __LAMBDA_SOCKETS__
#define __LAMBDA_SOCKETS__

#include "../lambda.hpp"
#include "../http/http.hpp"
#include <thread>
#include <mutex>

#ifndef SOCKET
	typedef uint64_t SOCKET;
#endif

#ifndef INVALID_SOCKET
	#define INVALID_SOCKET (-1)
#endif

namespace Lambda::Network {

	static const size_t network_chunksize_header = 2048;
	static const size_t network_chunksize_body = 131072;
	static const size_t network_chunksize_websocket = 1024;

	/**
	 * Send http response back to the client
	 * 😵 This function throws if a fatal error has occured
	*/
	Lambda::Error sendHTTP(SOCKET hSocket, HTTP::Response& response);

	/**
	 * Receive http request from the client
	 * 😵 This function throws if a fatal error has occured
	*/
	HTTP::Request receiveHTTP(SOCKET hSocket);


	struct WebsocketMessage {
		std::string message;
		time_t timestamp = 0;
		bool binary = false;
		bool partial = false;
	};

	class WebSocket {
		private:
			SOCKET hSocket = INVALID_SOCKET;
			std::vector<WebsocketMessage> rxQueue;
			std::thread* receiveThread = nullptr;

			void asyncReceive();

			std::mutex mtLock;

			Lambda::Error _sendMessage(const uint8_t* dataBuff, const size_t dataSize, bool binary);

		public:
			//WebSocket(SOCKET hParentSocket);
			WebSocket(SOCKET hTCPSocket, Lambda::HTTP::Request& initalRequest);
			~WebSocket();

			Lambda::Error sendMessage(const std::string& message) {
				return _sendMessage((const uint8_t*)message.data(), message.size(), false);
			}
			Lambda::Error sendMessage(const std::vector<uint8_t>& message) {
				return _sendMessage(message.data(), message.size(), true);
			}
			bool availableMessage() { return this->rxQueue.size() > 0; };
			std::vector<WebsocketMessage> getMessages();
	};

	class HTTPConnection {
		private:
			SOCKET hSocket = INVALID_SOCKET;
			std::string clientIPv4;

		public:
			/**
			 * Constructs ready to use HTTP socket. Basically, establishes an http connection
			 * 😵 This constructor throws if unable to establish a connection
			*/
			HTTPConnection(SOCKET hParentSocket, time_t timeoutMs);
			HTTPConnection(SOCKET hParentSocket) : HTTPConnection(hParentSocket, 15000) {}
			~HTTPConnection();

			std::string clientIP() { return this->clientIPv4; }

			/**
			 * Sends http response back to the client
			 * 😵 This function throws if a fatal error has occured
			*/
			Lambda::Error sendMessage(Lambda::HTTP::Response& response) { return sendHTTP(this->hSocket, response); }

			/**
			 * Receives http request from the client
			 * 😵 This function throws if a fatal error has occured
			*/
			Lambda::HTTP::Request receiveMessage() { return receiveHTTP(this->hSocket); }

			WebSocket upgradeToWebsocket(Lambda::HTTP::Request& initalRequest) { return WebSocket(this->hSocket, initalRequest); }
	};

	class ListenSocket {
		private:
			SOCKET hSocket = INVALID_SOCKET;

		public:
			/**
			 * Constructs a listening socket for TCP stuff
			 * 😵 This constructor throws if unable to create socket
			*/
			ListenSocket() : ListenSocket("8080") {};
			ListenSocket(const char* listenPort);
			~ListenSocket();

			bool isAlive();

			HTTPConnection acceptConnection() { return HTTPConnection(this->hSocket); };
			//WebSocket acceptWebsocket() { return WebSocket(this->hSocket); }
	};

};

#endif
