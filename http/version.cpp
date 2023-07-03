#include "./http.hpp"

using namespace Lambda::HTTP;

HttpVersion::HttpVersion(const std::string& version) {

	std::string tmpBuffMajor;
	std::string tmpBuffMinor;

	uint8_t stage = 0;

	for (const auto& c : version) {

		if (c == '.') stage++;
		if (c < '0' || c > '9') continue;

		switch (stage) {
			case 1: {
				tmpBuffMinor.push_back(c);
			} break;
		
			default: {
				tmpBuffMinor.push_back(c);
			} break;
		}
	}

	try {
		if (tmpBuffMajor.size()) this->major = std::stoi(tmpBuffMajor);
		if (tmpBuffMinor.size()) this->minor = std::stoi(tmpBuffMinor);
	} catch(...) {
		//	whatever
	}
}
