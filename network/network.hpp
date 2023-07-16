#ifndef __LAMBDA_SOCKETS__
#define __LAMBDA_SOCKETS__

#include "../lambda_private.hpp"
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

	class WebSocket;
	class HTTPConnection;

	static const size_t network_chunksize_header = 2048;
	static const size_t network_chunksize_body = 131072;
	static const size_t network_chunksize_websocket = UINT16_MAX;
	static const size_t network_dnsresolve_attempts = 5;
	static const time_t network_connection_timeout = 30000;

	enum struct ConnectionProtocol : uint16_t {
		TCP = 0,
		UDP = 1
	};

	class ListenSocket {
		protected:
			SOCKET hSocket = INVALID_SOCKET;
			void create(uint16_t listenPort, ConnectionProtocol proto);

		public:
			ListenSocket() {};
			~ListenSocket();

			bool isAlive();
	};

	class TCPListenSocket : public ListenSocket {
		public:
			/**
			 * Constructs a listening socket for TCP stuff
			 * 😵 This constructor throws if unable to create socket
			*/
			TCPListenSocket(uint16_t listenPort);

			HTTPConnection acceptConnection();
	};

	class BaseConnection {
		protected:
			SOCKET hSocket = INVALID_SOCKET;
			std::string peerIPv4;
			void resolveAndConnect(const char* host, const char* port, ConnectionProtocol proto);

		public:
			BaseConnection() {};
			~BaseConnection();

			void setTimeouts(uint32_t timeoutMs);
			SOCKET getHandle() noexcept;

			BaseConnection& operator= (const BaseConnection& other) = delete;
			BaseConnection& operator= (BaseConnection&& other) noexcept;
			BaseConnection(BaseConnection&& other) noexcept;

			const std::string& getPeerIPv4() noexcept;
	};

	class HTTPConnection : public BaseConnection {
		public:
			HTTPConnection() {};
			HTTPConnection(SOCKET hParentSocket);
			HTTPConnection(HTTP::URL remoteUrl);

			/**
			 * Return http response to the client
			*/
			void sendResponse(HTTP::Response& response);

			/**
			 * Send http request to the server
			*/
			void sendRequest(HTTP::Request& request);

			/**
			 * Receive http request from the client
			 * 😵 This function throws if a fatal error has occured
			*/
			HTTP::Request receiveRequest();

			/**
			 * Receive http response from the server
			 * 😵 This function throws if a fatal error has occured
			*/
			HTTP::Response receiveResponse();

			/**
			 * Upgrate the connection to websocket
			 * 😵 This function throws if a fatal error has occured
			*/
			WebSocket upgradeToWebsocket(Lambda::HTTP::Request& initalRequest);
	};

	enum struct WebSocketCloseCode : uint16_t {
		normal = 1000,
		going_away = 1001,
		protocol_error = 1002,
		unsupported_data = 1003,
		no_status_received = 1005,
		abnormal_close = 1006,
		invalid_payload_data = 1007,
		policy_violation = 1008,
		message_too_big = 1009,
		mandatory_extension = 1010,
		internal_server_error = 1011,
		tls_handshake_failed = 1015
	};

	struct WebsocketMessage {
		std::string content;
		time_t timestamp = 0;
		bool binary = false;
	};

	class WebSocket {
		private:
			SOCKET hSocket = INVALID_SOCKET;
			std::vector<WebsocketMessage> rxQueue;
			std::thread receiveThread;
			void asyncWsIO();
			Lambda::Error internalError;
			std::mutex mtLock;
			uint16_t connCloseStatus = 0;

		public:

			/**
			 * Create websocket by accepting incoming request
			*/
			WebSocket(HTTPConnection& connection, HTTP::Request& initalRequest);

			/**
			 * Create websocket by creating a request
			*/
			//WebSocket(HTTPConnection& connection);

			~WebSocket();

			Lambda::Error sendMessage(const uint8_t* dataBuff, const size_t dataSize, bool binary);
			Lambda::Error sendMessage(const std::string& message);

			bool availableMessage();
			std::vector<WebsocketMessage> getMessages();

			bool isAlive();
			Lambda::Error getError();

			void close();
			void close(WebSocketCloseCode reason);
	};

};

#endif
