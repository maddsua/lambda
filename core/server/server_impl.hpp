#ifndef __LIB_MADDSUA_LAMBDA_CORE_SERVER_IMPL__
#define __LIB_MADDSUA_LAMBDA_CORE_SERVER_IMPL__

#include "./server.hpp"
#include "../network/tcp/connection.hpp"

#include <string>
#include <thread>

namespace Lambda::Server {

	enum struct HandlerMode {
		HTTP, WS, SSE
	};

	void connectionWorker(WorkerContext& workerctx, const ServeOptions& config, const RequestCallback& handlerCallback) noexcept;

	namespace Pages {
		HTTP::Response renderErrorPage(HTTP::Status code, const std::string& message, ErrorResponseType type);
		HTTP::Response renderErrorPage(HTTP::Status code, const std::string& message);
	};

	namespace Connection {

		/**
		 * UpgradeError is thrown whenever a transport context fails to create a subprotocol
		 * (like Websocker or start SSE stream) 
		*/
		class UpgradeError : public Lambda::Error {
			private:
				const HTTP::Status m_status = 400;

			public:
				UpgradeError(const std::string& message) : Error(message) {}
				UpgradeError(const std::string& message, HTTP::Status status) : Error(message), m_status(status) {}

				const HTTP::Status& status() const noexcept { return this->m_status; }
		};
	};
};

#endif
