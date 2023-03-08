#ifndef _maddsua_base64
#define _maddsua_base64


#include <stdint.h>
#include <string>
#include <vector>


namespace maddsua {

	std::string b64Decode(std::string* data);
	std::string b64Encode(std::string* data);

	bool b64Validate(const std::string* encoded);
	
}

#endif