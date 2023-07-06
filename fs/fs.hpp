#ifndef __LAMBDA_FS__
#define __LAMBDA_FS__

#include "../lambda.hpp"
#include <vector>
#include <string>

namespace Lambda::Fs {

	Lambda::Error readFileSync(std::string path, std::vector<uint8_t>& content);
	Lambda::Error readTextFileSync(std::string path, std::string& content);

	Lambda::Error writeFileSync(std::string path, std::vector<uint8_t>& content);
	Lambda::Error writeTextFileSync(std::string path, std::string& content);

	void normalizePath(std::string& path);

};

#endif
