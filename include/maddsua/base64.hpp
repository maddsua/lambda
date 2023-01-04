
#include <stdint.h>
#include <string>
#include <vector>

#ifndef _maddsua_base64
#define _maddsua_base64

namespace maddsua {

	std::vector <uint8_t> b64Decode(std::string encoded);
	std::string b64Encode(std::vector <uint8_t> source);
	bool b64Validate(std::string* encoded);

}

#endif