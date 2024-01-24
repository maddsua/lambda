/**
	An http and websocket C++ library
	Or at least, it apperas to be
	2023 maddsua, https://github.com/maddsua

	Congrats! This is the entry point. You're good to go!
*/

#ifndef __LIB_MADDSUA_LAMBDA_ENTRYPOINT__
#define __LIB_MADDSUA_LAMBDA_ENTRYPOINT__

#include "./lambda_version.hpp"

#include "./core/core.hpp"
#include "./extra/extra.hpp"

namespace Lambda {
	typedef HTTP::Request Request;
	typedef HTTP::Response Response;
	typedef Endpoints::RequestContext Context;
};

#endif
