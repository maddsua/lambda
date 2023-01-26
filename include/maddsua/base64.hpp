#ifndef _maddsua_base64
#define _maddsua_base64


#include <stdint.h>
#include <string>
#include <vector>


namespace maddsua {

	std::string b64Decode(std::string encoded);
	std::string b64Encode(std::string source);
	bool b64Validate(std::string* encoded);

}

#endif