#ifndef __LIB_MADDSUA_LAMBDA_CORE_SERVER__
#define __LIB_MADDSUA_LAMBDA_CORE_SERVER__

#include "./options.hpp"
#include "./handlers.hpp"
#include "./worker.hpp"

#include "../network/tcp/listener.hpp"

#include <forward_list>

namespace Lambda {

	class LambdaInstance {
		private:
			Network::TCP::ListenSocket listener;
			ServerConfig config;
			RequestCallback httpHandler;

			std::future<void> acceptWorker;
			std::forward_list<WorkerContext> m_connections;
			bool m_terminated = false;

			void terminate();

		public:
			LambdaInstance(RequestCallback handlerCallback, ServerConfig init);
			~LambdaInstance();

			void shutdownn();
			void awaitFinished();

			const ServerConfig& getConfig() const noexcept;
	};
};

#endif
