#ifndef __LIB_MADDSUA_LAMBDA_CORE_SERVER__
#define __LIB_MADDSUA_LAMBDA_CORE_SERVER__

#include "./options.hpp"
#include "./handlers.hpp"

#include "../network/tcp/listener.hpp"

namespace Lambda {

	class LambdaInstance {
		private:

			Network::TCP::ListenSocket listener;
			ServerConfig config;
			RequestCallback httpHandler;

			std::future<void> watchdogWorker;
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
