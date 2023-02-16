/*
	maddsua's lambda
	A C++ HTTP server framework
	2023 https://github.com/maddsua/lambda

	Header: base 64 encoder/decoder functions
*/


#ifndef H_MADDSUA_BASE64
#define H_MADDSUA_BASE64


#include <stdint.h>
#include <string>
#include <vector>


namespace maddsua {

	std::string b64Decode(std::string* data);
	std::string b64Encode(std::string* data);

	bool b64Validate(const std::string* encoded);
	
}

#endif