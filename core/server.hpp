#ifndef __LIB_MADDSUA_LAMBDA_CORE_SERVER__
#define __LIB_MADDSUA_LAMBDA_CORE_SERVER__

#include <functional>

#include "./http.hpp"
#include "./network.hpp"

namespace Lambda {

	namespace Server {
		typedef std::function<HTTP::Response(const HTTP::Request request, const Network::ConnInfo& info)> HttpHandlerFunction;
		void handleHTTPConnection(Network::TCPConnection& conn, HttpHandlerFunction handler);
	};

};

#endif
