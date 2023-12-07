#ifndef _OCTOPUSS_CORE_ENCODING_
#define _OCTOPUSS_CORE_ENCODING_

#include "./polyfill.hpp"
#include "./http.hpp"

namespace Encoding {

	std::string toBase64(const std::string& data);
	std::string fromBase64(const std::string& data);

};

#endif
