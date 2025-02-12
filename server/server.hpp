#ifndef __LIB_MADDSUA_LAMBDA_SERVER__
#define __LIB_MADDSUA_LAMBDA_SERVER__

#include <future>
#include <functional>
#include <memory>

#include "../net/net.hpp"
#include "../http/http.hpp"

namespace Lambda {

	struct ServeOptions {
		int16_t port = 8000;
		std::string host_addr;
		bool debug = false;
		size_t max_header_size = 1024 * 1024;
		size_t max_body_size = 10 * 1024 * 1024;
	};

	class ServeContext;

	class Server {
		protected:
			HandlerFn m_handler_fn;
			std::shared_ptr<Handler> m_handler;
			Net::TcpListener m_tcp;
			std::future<void> m_loop;
			bool m_active = false;
			std::shared_ptr<bool> m_exit = std::make_shared<bool>(false);

		public:

			static const uint32_t DefaultIoTimeout = 15000;

			Server(HandlerFn handler_fn);
			Server(std::shared_ptr<Handler> handler);
			Server(HandlerFn handler_fn, ServeOptions options);
			Server(std::shared_ptr<Handler> handler, ServeOptions options);

			Server(const Server& other) = delete;
			~Server();

			ServeOptions options;

			void serve();
			void shutdown();
	};

	class ServeContext {
		private:
			std::shared_ptr<bool> m_done;
		public:
			ServeContext(const ServeOptions& opts, std::shared_ptr<bool> done)
				: m_done(done), opts(opts) {}

			const ServeOptions opts;

			bool done() const noexcept {
				return *this->m_done;
			}
	};
};

#endif
