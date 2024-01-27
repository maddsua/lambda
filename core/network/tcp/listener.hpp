#ifndef __LIB_MADDSUA_LAMBDA_NETWORK_TCP_LISTEN__
#define __LIB_MADDSUA_LAMBDA_NETWORK_TCP_LISTEN__

#include "./connection.hpp"
#include "../sysnetw.hpp"

#include <optional>

namespace Lambda::Network::TCP {

	struct ListenConfig {
		bool allowPortReuse = false;
		uint16_t port = 8180;
		uint32_t connectionTimeout = 15000;
	};

	class ListenSocket {
		protected:
			SOCKET hSocket = INVALID_SOCKET;
			ListenConfig config;

		public:
			ListenSocket() = default;
			ListenSocket(const ListenConfig& init);
			ListenSocket(const ListenSocket& other) = delete;
			ListenSocket(ListenSocket&& other) noexcept;
			~ListenSocket();

			ListenSocket& operator=(const ListenSocket& other) = delete;
			ListenSocket& operator=(ListenSocket&& other) noexcept;

			std::optional<Connection> acceptConnection();

			bool active() const noexcept;
			void stop() noexcept;
			const ListenConfig& getConfig() const noexcept;
	};
};

#endif
