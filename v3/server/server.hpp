#ifndef __LAMBDA_SERVER__
#define __LAMBDA_SERVER__

#include <future>
#include <functional>

#include "../net/net.hpp"
#include "../http/http.hpp"

namespace Lambda {

	struct ServeOptions {
		int16_t port = 8000;
		size_t max_header_size = 1024 * 1024;
		size_t max_body_size = 10 * 1024 * 1024;
		bool debug = false;
		//	todo: add binding opts and stuff
	};

	class ServerContext;

	class Server {
		protected:
			HandlerFn m_handler;
			Net::TcpListener m_tcp;
			std::future<void> m_loop;
			bool m_active = false;
			bool m_exit = false;

		public:

			static const uint32_t DefaultIoTimeout = 15000;

			Server(HandlerFn handler) : m_handler(handler) {}
			Server(HandlerFn handler, ServeOptions options) : m_handler(handler), options(options) {}
			Server(const Server& other) = delete;
			~Server();

			ServeOptions options;

			void Serve();
			void Shutdown();

			friend class ServerContext;
	};

	class ServerContext {
		private:
			const Server* m_parent;
		public:
			ServerContext(const Server* parent) : m_parent(parent) {}

			bool done() const noexcept {
				return this->m_parent->m_exit;
			}

			ServeOptions options() const noexcept {
				return this->m_parent->options;
			}
	};
};

#endif
