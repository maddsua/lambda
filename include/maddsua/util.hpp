#ifndef _lambdautils
#define _lambdautils

#include <stdint.h>
#include <string>
#include <vector>

	namespace maddsua {
		
		std::string binToHex(const uint8_t* data, const size_t length);

		inline std::string binToHex(const std::string data) {
			return binToHex((const uint8_t*)data.data(), data.size());
		}
		inline std::string binToHex(std::vector <uint8_t> data) {
			return binToHex(data.data(), data.size());
		}

		std::vector <uint8_t> hexToBin(std::string& data);
	}

#endif