/*
	base 64 encoder/decoder functions
	2023 maddsua
	https://github.com/maddsua/
*/


#ifndef H_MADDSUA_BASE64
#define H_MADDSUA_BASE64

	#include <stdint.h>
	#include <string>
	#include <vector>

	namespace lambda {

		std::string b64Decode(std::string* data);
		std::string b64Encode(std::string* data);

		bool b64Validate(const std::string* encoded);
		
	}

#endif