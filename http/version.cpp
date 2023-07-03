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
		this->major = tmpBuffMajor.size() ? std::stoi(tmpBuffMajor) : 1;
		this->minor = tmpBuffMinor.size() ? std::stoi(tmpBuffMajor) : 0;
	} catch(...) { /* whatever*/ }
}

std::string HttpVersion::toString() {
	return "HTTP/" + std::to_string(this->major) + (this->minor > 0 ? ("." + std::to_string(this->minor)) : "");
}
