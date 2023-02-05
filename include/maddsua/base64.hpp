#ifndef _maddsua_base64
#define _maddsua_base64


#include <stdint.h>
#include <string>
#include <vector>


namespace maddsua {

	void b64Decode(std::string* encoded, std::string* plain);
	void b64Encode(std::string* plain, std::string* encoded);

	inline std::string b64Decode(std::string encoded) {
		std::string result;
		b64Decode(&encoded, &result);
		return result;
	}

	inline std::string b64Encode(std::string source) {
		std::string result;
		b64Encode(&source, &result);
		return result;
	}

	inline bool b64Validate(std::string* encoded) {
		
		for (auto bchr : *encoded) {
			if (!isalnum(bchr) && !((bchr == '+') || (bchr == '/') || (bchr == '=')))
				return false;
		}
			
		return true;
	}

}

#endif