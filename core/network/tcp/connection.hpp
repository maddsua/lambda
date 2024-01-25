#ifndef __LIB_MADDSUA_LAMBDA_NETWORK_TCP_CONNECTION__
#define __LIB_MADDSUA_LAMBDA_NETWORK_TCP_CONNECTION__

#include <vector>
#include <string>
#include <mutex>

#include "../network.hpp"

namespace Lambda::Network::TCP {

	struct ConnCreateInit {
		SOCKET hSocket = INVALID_SOCKET;
		ConnectionInfo info;
	};

	enum struct SetConnectionTimeoutDirection {
		Both, Rx, Tx
	};

	class Connection {
		protected:
			SOCKET hSocket = INVALID_SOCKET;
			ConnectionInfo info;
			std::mutex readMutex;
			std::mutex writeMutex;

		public:
			Connection(ConnCreateInit init);
			Connection(Connection&& other) noexcept;
			~Connection();

			Connection& operator= (const Connection& other) = delete;
			Connection& operator= (Connection&& other) noexcept;

			std::vector<uint8_t> read();
			std::vector<uint8_t> read(size_t expectedSize);
			void write(const std::vector<uint8_t>& data);
			const ConnectionInfo& getInfo() const noexcept;
			void closeconn();
			bool ok() const noexcept;

			void setTimeouts(uint32_t value);
			void setTimeout(uint32_t value, SetConnectionTimeoutDirection direction);

			static const uint32_t TimeoutMs_Default = 15000;
			static const uint32_t TimeoutMs_Max = 60000;
			static const uint32_t TimeoutMs_Min = 100;
			static const uint32_t ReadChunkSize = 2048;
	};
};

#endif
