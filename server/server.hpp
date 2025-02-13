#ifndef __LIB_MADDSUA_LAMBDA_SERVER__
#define __LIB_MADDSUA_LAMBDA_SERVER__

#include <future>
#include <functional>

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

	class Server {
		protected:
			HandlerFn m_handler_fn;
			std::shared_ptr<Handler> m_handler;
			Net::TcpListener m_tcp;
			std::future<void> m_loop;
			std::shared_ptr<bool> m_done = std::make_shared<bool>(true);

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

};

#endif
