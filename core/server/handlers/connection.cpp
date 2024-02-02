#include "./handlers.hpp"
#include "../../polyfill/polyfill.hpp"

using namespace Lambda;
using namespace Lambda::Server;
using namespace Lambda::Server::Handlers;

void Handlers::connectionHandler(
	Network::TCP::Connection&& conn,
	const ServeOptions& config,
	const ConnectionCallback& handlerCallback
) noexcept {

	auto createLogTimeStamp = [&]() {
		if (config.loglevel.timestamps) {
			return Date().toHRTString() + ' ';
		}
		return std::string();
	};

	const auto& conninfo = conn.info();

	if (config.loglevel.connections) fprintf(stdout,
		"%s%s:%i connected on %i\n",
		createLogTimeStamp().c_str(),
		conninfo.remoteAddr.hostname.c_str(),
		conninfo.remoteAddr.port,
		conninfo.hostPort
	);

	try {

		auto connctx = IncomingConnection(conn, config.transport);
		handlerCallback(connctx);

	} catch(const std::exception& e) {

		if (config.loglevel.requests) fprintf(stderr,
			"%s[Service] Connection to %s terminated: %s\n",
			createLogTimeStamp().c_str(),
			conninfo.remoteAddr.hostname.c_str(),
			e.what()
		);

	} catch(...) {

		if (config.loglevel.requests) fprintf(stderr,
			"%s[Service] Connection to %s terminated (unknown error)\n",
			createLogTimeStamp().c_str(),
			conninfo.remoteAddr.hostname.c_str()
		);
	}

	if (config.loglevel.connections) fprintf(stdout,
		"%s%s:%i disconnected from %i\n",
		createLogTimeStamp().c_str(),
		conninfo.remoteAddr.hostname.c_str(),
		conninfo.remoteAddr.port,
		conninfo.hostPort
	);
}
