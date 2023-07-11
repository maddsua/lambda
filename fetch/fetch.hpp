/**
 * While fetch API is technically a part of network component,
 * it's functionality is too different from the main component.
 * Well, it's a client-side component, and most of the network stuff
 * is oriented towards making network work in the first place, not pulling REST API or something
*/

#ifndef __LAMBDA_FETCHAPI__
#define __LAMBDA_FETCHAPI__

#include "../http/http.hpp"

namespace Lambda::Fetch {

	/**
	 * Fetch some http resource. Works similar to JS Fetch API
	 * 
	 * https is not supported so it's only useful to interact with local services.
	 * 
	 * Use libcurl for connecting to the world wide web
	*/
	HTTP::Response fetch(const HTTP::Request& userRequest);
	HTTP::Response fetch(std::string url);

};

#endif
