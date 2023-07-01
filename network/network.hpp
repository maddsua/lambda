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
	static const size_t network_chunksize_websocket = UINT16_MAX;

	Lambda::Error sendHTTP(SOCKET hSocket, HTTP::Response& response);

	/**
	 * Receive http request from the client
	 * 😵 This function throws if a fatal error has occured
	*/
	HTTP::Request receiveHTTP(SOCKET hSocket);

	enum WebSocketCloseCode {
		WSCLOSE_NORMAL = 1000,
		WSCLOSE_GOING_AWAY = 1001,
		WSCLOSE_PROTOCOL_ERROR = 1002,
		WSCLOSE_UNSUPPORTED_DATA = 1003,
		WSCLOSE_NO_STATUS_RECEIVED = 1005,
		WSCLOSE_ABNORMAL_CLOSE = 1006,
		WSCLOSE_INVALID_PAYLOAD_DATA = 1007,
		WSCLOSE_POLICY_VIOLATION = 1008,
		WSCLOSE_MESSAGE_TOO_BIG = 1009,
		WSCLOSE_MANDATORY_EXTENSION = 1010,
		WSCLOSE_INTERNAL_SERVER_ERROR = 1011,
		WSCLOSE_TLS_HANDSHAKE_FAILED = 1015
	};

	struct WebsocketMessage {
		std::string content;
		time_t timestamp = 0;
		bool binary = false;
	};

	struct WebsocketFrameHeader {
		size_t payloadSize;
		size_t size;
		uint8_t maskKey[4];
		uint8_t opcode;
		bool finbit;
		bool mask;
	};

	class WebSocket {
		private:
			SOCKET hSocket = INVALID_SOCKET;
			std::vector<WebsocketMessage> rxQueue;
			std::thread* receiveThread = nullptr;
			void asyncWsIO();
			Lambda::Error internalError;
			std::mutex mtLock;
			Lambda::Error _sendMessage(const uint8_t* dataBuff, const size_t dataSize, bool binary);
			uint16_t connCloseStatus = 0;
			WebsocketFrameHeader parseFrameHeader(const std::vector<uint8_t>& buffer);

		public:
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

			bool isAlive() { return (!this->connCloseStatus && this->hSocket != INVALID_SOCKET && !this->internalError.isError()); };
			Lambda::Error getError() { return this->internalError; };

			void close(WebSocketCloseCode status) { this->connCloseStatus = status; };
			void close() { close(WSCLOSE_NORMAL); };
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
			bool isAlive() { return this->hSocket != INVALID_SOCKET; };

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

			/**
			 * Upgrate the connection to websocket
			 * 😵 This function throws if a fatal error has occured
			*/
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

			bool isAlive() { return this->hSocket != INVALID_SOCKET; };

			HTTPConnection acceptConnection() { return HTTPConnection(this->hSocket); };
	};

};

#endif
