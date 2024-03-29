#ifndef __LIB_MADDSUA_LAMBDA_NETWORK_TCP_CONNECTION__
#define __LIB_MADDSUA_LAMBDA_NETWORK_TCP_CONNECTION__

#include "../network.hpp"

#include <vector>
#include <string>
#include <mutex>

namespace Lambda::Network::TCP {

	struct ConnectionFlags {
		/**
		 * Close connection when read operation times out
		*/
		bool closeOnTimeout = true;
		/**
		 * Throw a network error when read operation is terminated by disconnect
		 * 
		 * Note: socket will be closed either way
		*/
		bool throwOnDisconnect = true;
	};

	class Connection {
		protected:
			SockHandle hSocket;
			ConnectionInfo m_info;
			std::mutex m_read_mtx;
			std::mutex m_write_mtx;

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
			bool isOpen() const noexcept;

			void setTimeouts(uint32_t valueMs, SetTimeoutsDirection direction);
			void setTimeouts(uint32_t valueMs);

			static const uint32_t TimeoutMs = 15000;
			static const uint32_t DefaultChunkSize = 2048;

			/**
			 * Connection configuration flags
			*/
			ConnectionFlags flags;
	};
};

#endif
