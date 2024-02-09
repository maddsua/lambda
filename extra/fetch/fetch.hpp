/**
 * While fetch API is technically a part of network component,
 * it's functionality is too different from the main component.
 * Well, it's a client-side component, and most of the network stuff
 * is oriented towards making network work in the first place, not pulling REST API or something
*/

#ifndef __LIB_MADDSUA_LAMBDA_EXTRA_FETCH_API__
#define __LIB_MADDSUA_LAMBDA_EXTRA_FETCH_API__

#include "../../core/http/http.hpp"

namespace Lambda {

	namespace FetchAPI {

		struct RequestInit {
			HTTP::Method methood;
			HTTP::Headers headers;
			HTTP::BodyBuffer body;
		};
	};

	HTTP::Response fetch(const std::string& url, const FetchAPI::RequestInit& init);
	HTTP::Response fetch(const std::string& url);

};

#endif
