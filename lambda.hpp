
/*
	An http and websocket C++ framework

	https://github.com/maddsua/lambda
*/

#ifndef __LIB_MADDSUA_LAMBDA__
#define __LIB_MADDSUA_LAMBDA__

	#include "./version.hpp"
	#include "./buildopts.hpp"

	//	include core "modules"
	#include "./core/http/http.hpp"
	#include "./core/server/server.hpp"
	#include "./core/polyfill/polyfill.hpp"
	#include "./core/json/json.hpp"
	#include "./core/html/html.hpp"
	#include "./core/encoding/encoding.hpp"
	#include "./core/crypto/crypto.hpp"
	#include "./core/utils/utils.hpp"

	#ifdef LAMBDA_BUILDOPTS_ENABLE_COMPRESSION
		#include "./core/compression/compression.hpp"
	#endif

	//	include extra "modules"
	#include "./extra/kvstorage/kvstorage.hpp"
	#include "./extra/staticserver/staticserver.hpp"
	#include "./extra/vfs/vfs.hpp"

	namespace Lambda {
		typedef HTTP::Request Request;
		typedef HTTP::Response Response;
		typedef RequestContext Context;
		typedef Storage::LocalStorage LocalStorage;
	};

#endif
