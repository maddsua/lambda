#ifndef __LIB_MADDSUA_LAMBDA_NETWORK_TCP_CONNECTION__
#define __LIB_MADDSUA_LAMBDA_NETWORK_TCP_CONNECTION__

#include "../network.hpp"

#include <vector>
#include <string>
#include <mutex>

namespace Lambda::Network::TCP {

	class Connection {
		protected:
			SockHandle hSocket;
			ConnectionInfo m_info;
			std::mutex m_readMutex;
			std::mutex m_writeMutex;

		public:

			Connection(SockHandle handleInit, const ConnectionInfo& infoInit);
			Connection(Connection&& other) noexcept;
			Connection(const Connection& other) = delete;
			~Connection();

			Connection& operator=(const Connection& other) = delete;
			Connection& operator=(Connection&& other) noexcept;

			std::vector<uint8_t> read();
			std::vector<uint8_t> read(size_t expectedSize);
			void write(const std::vector<uint8_t>& data);
			const ConnectionInfo& info() const noexcept;
			void end() noexcept;
			bool active() const noexcept;

			void setTimeouts(uint32_t value, SetTimeoutsDirection direction);
			void setTimeouts(uint32_t value);

			static const uint32_t TimeoutMs = 15000;
			static const uint32_t ReadChunkSize = 2048;

			struct {
				bool closeOnTimeout = true;
			} flags;
	};
};

#endif
