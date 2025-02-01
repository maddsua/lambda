#ifndef __LIB_MADDSUA_LAMBDA_CORE_HTTP_TRANSPORT_IMPL__
#define __LIB_MADDSUA_LAMBDA_CORE_HTTP_TRANSPORT_IMPL__

#include "./http.hpp"
#include "./transport.hpp"

#include "../crypto/crypto.hpp"
#include "../network/tcp/connection.hpp"
#include "../utils/utils.hpp"

#include <vector>
#include <string>

namespace Lambda::HTTP::Transport {

	enum struct KeepAliveStatus {
		Unknown, KeepAlive, Close
	};

	class TransportContextV1 : public TransportContext {
		private:
			Net::TCP::Connection& m_conn;
			const TransportOptions& m_topts;

			const Crypto::ShortID m_id;

			std::vector<uint8_t> m_readbuff;
			KeepAliveStatus m_keepalive = KeepAliveStatus::Unknown;
			ContentEncodings m_compress = ContentEncodings::None;
			IncomingRequest* m_next = nullptr;

		public:
			TransportContextV1(Net::TCP::Connection& connInit, const TransportOptions& optsInit);

			const std::string& contextID() const noexcept;

			Net::TCP::Connection& tcpconn() const noexcept;
			const Net::ConnectionInfo& conninfo() const noexcept;
			const TransportOptions& options() const noexcept;
			const ContentEncodings& getEnconding() const noexcept;
			bool isConnected() const noexcept;

			bool awaitNext();
			IncomingRequest nextRequest();
			void respond(const ResponseContext& responsectx);

			std::vector<uint8_t> readRaw();
			std::vector<uint8_t> readRaw(size_t expectedSize);
			void writeRaw(const std::vector<uint8_t>& data);

			void reset() noexcept;
			bool hasPartialData() const noexcept;
			void close();
	};
};

#endif
