#ifndef __LIB_MADDSUA_LAMBDA_NETWORK_TCP_LISTEN__
#define __LIB_MADDSUA_LAMBDA_NETWORK_TCP_LISTEN__

#include "../compat.hpp"
#include "./connection.hpp"

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
			ListenSocket(const ListenConfig& init);
			ListenSocket(ListenSocket&& other);
			~ListenSocket();

			ListenSocket& operator=(const ListenSocket& other) = delete;
			ListenSocket& operator=(ListenSocket&& other) noexcept;

			Connection acceptConnection();

			bool ok() const noexcept;
			const ListenConfig& getConfig() const noexcept;
	};
};

#endif
