#ifndef __LAMBDA_ENCODING__
#define __LAMBDA_ENCODING__

#include <string>
#include <vector>

namespace Lambda::Encoding {

	//	URL-encode string
	std::string encodeURIComponent(const std::string& str);

	//	Decode base64 string
	std::string b64Decode(const std::string& data);
	//	Encode base64 string
	std::string b64Encode(const std::string& data);
	//	Validate base64 string
	bool b64Validate(const std::string& encoded);

	std::string hexEncode(std::string data);
	std::string hexEncode(std::vector<uint8_t> data);
	
};

#endif
