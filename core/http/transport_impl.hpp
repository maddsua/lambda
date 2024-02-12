#ifndef __LIB_MADDSUA_LAMBDA_CORE_HTTP_TRANSPORT_IMPL__
#define __LIB_MADDSUA_LAMBDA_CORE_HTTP_TRANSPORT_IMPL__

#include "./transport.hpp"

#include "../network/tcp/connection.hpp"
#include "./http.hpp"
#include "../utils/utils.hpp"

#include <vector>
#include <string>
#include <unordered_map>
#include <map>
#include <optional>

namespace Lambda::HTTP::Transport {

	class TransportContextV1 : public TransportContext {
		private:
			Network::TCP::Connection& m_conn;
			const TransportOptions& m_topts;

			std::vector<uint8_t> m_readbuff;
			bool m_keepalive = false;
			ContentEncodings m_compress = ContentEncodings::None;
			HTTP::Request* m_next = nullptr;

		public:
			TransportContextV1(Network::TCP::Connection& connInit, const TransportOptions& optsInit);

			Network::TCP::Connection& tcpconn() const noexcept;
			const Network::ConnectionInfo& conninfo() const noexcept;
			const TransportOptions& options() const noexcept;
			const ContentEncodings& getEnconding() const noexcept;
			bool ok() const noexcept;

			bool awaitNext();
			HTTP::Request nextRequest();
			void respond(const HTTP::Response& response);
			void reset() noexcept;
			bool hasPartialData() const noexcept;
	};
};

#endif
