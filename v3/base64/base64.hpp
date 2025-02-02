#ifndef __LAMBDA_ENCODING_BASE64__
#define __LAMBDA_ENCODING_BASE64__

#include <string>
#include <vector>
#include <cstdint>

namespace Lambda::Encoding::Base64 {

	std::string encode(const std::vector<uint8_t>& data);
	std::vector<uint8_t> decode(const std::string& data);
	bool validate(const std::string& data);

};

#endif
