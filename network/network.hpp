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

	/**
	 * Base class for listen socket
	*/
	class ListenSocket {
		protected:
			SOCKET hSocket = INVALID_SOCKET;

			/**
			 * Creates a listen socket for selected protocol
			*/
			void create(uint16_t listenPort, ConnectionProtocol proto);

		public:
			ListenSocket() {};
			~ListenSocket();

			/**
			 * True, if socket is fine
			*/
			bool isAlive();
	};

	/**
	 * TCP socket class
	*/
	class TCPListenSocket : public ListenSocket {
		public:
			/**
			 * Constructs a listening socket for TCP stuff
			 * 😵 This constructor throws if unable to create socket
			*/
			TCPListenSocket(uint16_t listenPort);

			/**
			 * Accept an http connection from this socket
			 * 
			 * This is a blocking operation, and this thread will be waiting for it to finish
			 * 
			 * 😵 This function throws if unable to accept a connection
			*/
			HTTPConnection acceptConnection();
	};

	/**
	 * Network connection base class
	*/
	class BaseConnection {
		protected:
			SOCKET hSocket = INVALID_SOCKET;
			std::string peerIPv4;

			/**
			 * Resolve host address and connect to a server on it
			 * 
			 * 😵 This function throws if unable to connect
			*/
			void resolveAndConnect(const char* host, const char* port, ConnectionProtocol proto);

			void connectLocalSerivce(uint16_t servicePort, ConnectionProtocol proto);

		public:
			/**
			 * Network connection base class
			*/
			BaseConnection() {};
			~BaseConnection();

			/**
			 * Set connection timeouts
			*/
			void setTimeouts(uint32_t timeoutMs);

			/**
			 * Returns underlying socket handle
			*/
			SOCKET getHandle() noexcept;

			/**
			 * This object is not copyable
			*/
			BaseConnection& operator= (const BaseConnection& other) = delete;

			/**
			 * This object is movable
			*/
			BaseConnection& operator= (BaseConnection&& other) noexcept;

			/**
			 * Get peer's IP v4 address
			*/
			const std::string& getPeerIPv4() noexcept;
	};

	/**
	 * HTTP connection class
	*/
	class HTTPConnection : public BaseConnection {
		public:

			/**
			 * Default empty constructor
			*/
			HTTPConnection() {};

			/**
			 * Creates an http connection by accepting it from listening socket
			 * 
			 * 😵 This constructor throws if unable to accept connection
			*/
			HTTPConnection(SOCKET hParentSocket);

			/**
			 * Creates an http connection by resolving URL and connecting to remote server
			 * 
			 * 😵 This constructor throws if unable to create connection
			*/
			HTTPConnection(HTTP::URL remoteUrl);

			/**
			 * Create HTTP connection to a service running on the same machine
			 * 
			 * 😵 This constructor throws if unable to create connection
			*/
			HTTPConnection(uint16_t servicePort);

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

	/**
	 * Websocket connection close status codes
	*/
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

			/**
			 * Send a message to this websocket
			*/
			Lambda::Error sendMessage(const uint8_t* dataBuff, const size_t dataSize, bool binary);

			/**
			 * Send text message (packet)
			*/
			Lambda::Error sendMessage(const std::string& message);

			/**
			 * True, is there's any unread messages (packets)
			*/
			bool availableMessage();

			/**
			 * Retrieve websocket messages (packets) from temp buffer
			*/
			std::vector<WebsocketMessage> getMessages();

			/**
			 * True, if this websocket is fine
			 * 
			 * It is faster than getError(), so use it in loops and such
			*/
			bool isAlive();

			/**
			 * Get error status for this websocekt
			*/
			Lambda::Error getError();

			/**
			 * Close websocket with default status code
			*/
			void close();

			/**
			 * Close websocket with custom status code
			*/
			void close(WebSocketCloseCode reason);
	};

};

#endif
