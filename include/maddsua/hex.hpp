#ifndef _maddsua_hexconvert
#define _maddsua_hexconvert

#include <stdint.h>
#include <string>
#include <vector>

	namespace maddsua {
		std::string binToHex(const uint8_t* data, const size_t length);

		inline std::string binToHex(const std::string data) {
			return binToHex((const uint8_t*)data.data(), data.size());
		}
		inline std::string binToHex(const std::vector <uint8_t> data) {
			return binToHex(data.data(), data.size());
		}

		std::vector <uint8_t> hexToBin(const std::string& data);


	}

#endif