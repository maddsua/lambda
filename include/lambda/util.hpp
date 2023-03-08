#include <string>
#include <vector>


//	base64 stuff
std::string b64Decode(std::string* data);
std::string b64Encode(std::string* data);
bool b64Validate(const std::string* encoded);

//	random generators
std::vector <uint64_t> randomSequence(const size_t cap, const size_t length);
std::vector <uint8_t> randomStream(const size_t length);

namespace lambda {
	//	more random generators but more specific
	std::string createUniqueId();
	std::string createPassword(size_t length, bool randomCase);
}
