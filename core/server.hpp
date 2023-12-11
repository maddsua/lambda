#ifndef _LAMBDA_CORE_SERVER_
#define _LAMBDA_CORE_SERVER_

#include <functional>

#include "./http.hpp"
#include "./network.hpp"

namespace Lambda {

	typedef std::function<HTTP::Response(const HTTP::Request request, const Network::ConnInfo& info)> HttpHandlerFunction;
	void handleHTTPConnection(Network::TCPConnection& conn, HttpHandlerFunction handler);

};

#endif
