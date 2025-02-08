#ifndef __LIB_MADDSUA_LAMBDA_NET_UTILS__
#define __LIB_MADDSUA_LAMBDA_NET_UTILS__

#include <cstdint>
#include <stdexcept>
#include <optional>
#include <vector>
#include <cstdint>

namespace Lambda::Net {

	#ifdef _WIN32
		typedef uint64_t SockHandle;
	#else
		typedef int32_t SockHandle;
	#endif

	class Error : public std::exception {
		protected:
			std::string m_msg;
			int m_os_code = 0;

		public:
			Error(const char* message) {
				this->m_msg.append("NetError: ");
				this->m_msg.append(message);
			}

			Error(const char* message, int os_code) {
				
				this->m_os_code = os_code;

				this->m_msg.append("NetError: ");
				this->m_msg.append(message);
				this->m_msg.append(" (os code: ");
				this->m_msg.append(std::to_string(this->m_os_code));
				this->m_msg.push_back(')');
			}

			const char* what() const noexcept override {
				return this->m_msg.c_str();
			}

			const int os_code() const noexcept {
				return this->m_os_code;
			}
	};

	struct ConnectionTimeouts {
		uint32_t read = 0;
		uint32_t write = 0;
	};

	struct ListenOptions {
		uint16_t port = 80;
		std::string bind_addr;
		bool fast_port_reuse = true;
	};

	enum struct Transport {
		TCP,
		UDP
	};

	struct RemoteAddress {
		std::string hostname;
		uint16_t port = 0;
		Transport transport;
	};

	class TcpConnection {
		protected:
			SockHandle m_sock;
			RemoteAddress m_remote_addr;
			ConnectionTimeouts m_timeouts;
			uint32_t m_id;
		
		public:
			TcpConnection(SockHandle sock, const RemoteAddress& remote);
			TcpConnection(TcpConnection&& other) noexcept;
			TcpConnection(const TcpConnection& other) = delete;
			~TcpConnection();

			const RemoteAddress& remote_addr() const noexcept;
			const ConnectionTimeouts& timeouts() const noexcept;

			std::vector<uint8_t> read(size_t chunk_size);
			size_t write(const std::vector<uint8_t>& data);

			void set_timeouts(ConnectionTimeouts timeouts);

			void close() noexcept;
			bool is_open() const noexcept;

			uint32_t id() const noexcept;
	};

	class TcpListener {
		protected:
			SockHandle m_sock;

		public:
			ListenOptions options;

			TcpListener() = default;
			TcpListener(const ListenOptions& opts) : options(opts) {}
			TcpListener(const TcpListener& other) = delete;
			TcpListener(TcpListener&& other) noexcept;
			~TcpListener();

			TcpListener& operator=(const TcpListener& other) = delete;

			void bind_and_listen();

			TcpConnection next();

			bool is_listening() const noexcept;
			void shutdown();
	};

};

#endif
