#ifndef __LIB_MADDSUA_LAMBDA_NETWORK_TCP_CONNECTION__
#define __LIB_MADDSUA_LAMBDA_NETWORK_TCP_CONNECTION__

#include <vector>
#include <string>
#include <mutex>

#include "../compat.hpp"

namespace Lambda::Network::TCP {

	enum struct Constants : uint32_t {
		Receive_ChunkSize = 2048,
		Connection_TimeoutMs = 15000,
		Connection_TimeoutMs_Max = 60000,
		Connection_TimeoutMs_Min = 100,
	};

	enum struct ConnectionType {
		TCP, UDP
	};

	struct ConnInfo {
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
			void end();
			bool isOpen() const noexcept;
	};
};

#endif
