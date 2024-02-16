#ifndef __LIB_MADDSUA_LAMBDA_CORE_SERVER__
#define __LIB_MADDSUA_LAMBDA_CORE_SERVER__

#include "./options.hpp"
#include "./handlers.hpp"
#include "./worker.hpp"

#include "../network/tcp/listener.hpp"

#include <forward_list>
#include <atomic>

namespace Lambda {

	/**
	 * Lambda server class (this is what you want)
	*/
	class LambdaInstance {
		private:
			Network::TCP::ListenSocket listener;
			ServerConfig config;
			RequestCallback httpHandler;

			std::future<void> serviceWorker;
			std::forward_list<WorkerContext> m_connections;
			std::atomic<size_t> m_connections_count = 0;
			bool m_terminated = false;

			/**
			 * Internal call that signals all workers to exit
			*/
			void terminate();

		public:
			LambdaInstance(RequestCallback handlerCallback, ServerConfig init);
			~LambdaInstance();

			/**
			 * Stop server
			*/
			void shutdownn();

			/**
			 * Block current thread until server exits
			*/
			void awaitFinished();

			/**
			 * Return current instance config
			*/
			const ServerConfig& getConfig() const noexcept;
	};
};

#endif
