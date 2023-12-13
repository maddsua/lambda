#ifndef __LIB_MADDSUA_LAMBDA_NETWORK__
#define __LIB_MADDSUA_LAMBDA_NETWORK__

#include <vector>
#include <string>
#include <mutex>

#include "../core/http.hpp"
#include "../core/polyfill.hpp"
#include "./network/compat.hpp"

namespace Lambda::Network {

	enum struct Constants : uint32_t {
		Receive_ChunkSize = 2048,
		Connection_TimeoutMs = 15000,
		Connection_TimeoutMs_Max = 60000,
		Connection_TimeoutMs_Min = 100,
	};

	struct ConnInfo {
		ShortID shortid;
		std::string peerIP;
		uint16_t port;
	};

	struct ConnCreateInit {
		SOCKET hSocket;
		ConnInfo info;
		uint32_t connTimeout = 0;
	};

	class TCPConnection {
		protected:
			SOCKET hSocket = INVALID_SOCKET;
			ConnInfo info;
			std::mutex readMutex;
			std::mutex writeMutex;

		public:
			TCPConnection(ConnCreateInit init);
			TCPConnection(TCPConnection&& other) noexcept;
			~TCPConnection();

			TCPConnection& operator= (const TCPConnection& other) = delete;
			TCPConnection& operator= (TCPConnection&& other) noexcept;

			std::vector<uint8_t> read();
			std::vector<uint8_t> read(size_t expectedSize);
			void write(const std::vector<uint8_t>& data);
			const ConnInfo& getInfo() const noexcept;
			void close();
			bool isOpen() const noexcept;
	};

	struct ListenInit {
		bool allowPortReuse = false;
	};

	class TCPListenSocket {
		protected:
			SOCKET hSocket = INVALID_SOCKET;
			uint16_t internalPort = 0;

		public:
			TCPListenSocket(TCPListenSocket&& other);
			TCPListenSocket(uint16_t listenPort, const ListenInit& init);
			~TCPListenSocket();

			TCPListenSocket& operator=(const TCPListenSocket& other) = delete;
			TCPListenSocket& operator=(TCPListenSocket&& other) noexcept;

			TCPConnection acceptConnection();

			bool ok() const noexcept;
			uint16_t getPort() const noexcept;
	};
};

#endif
