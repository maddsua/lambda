/**
	An http and websocket C++ library
	Or at least, it apperas to be
	2023 maddsua, https://github.com/maddsua

	Congrats! This is the entry point. You're good to go!
*/

#ifndef __LIB_MADDSUA_LAMBDA__
#define __LIB_MADDSUA_LAMBDA__

#include "./version.hpp"

//	include core "modules"
#include "./core/http/http.hpp"
#include "./core/server/server.hpp"
#include "./core/polyfill/polyfill.hpp"
#include "./core/json/json.hpp"
#include "./core/encoding/encoding.hpp"
#include "./core/crypto/crypto.hpp"
#include "./core/compression/compression.hpp"
#include "./core/error/error.hpp"
#include "./core/html/templates.hpp"

//	include extra "modules"
#include "./extra/storage/storage.hpp"

namespace Lambda {
	typedef HTTP::Request Request;
	typedef HTTP::Response Response;
	typedef RequestContext Context;
};

#endif
