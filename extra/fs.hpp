#ifndef  __LIB_MADDSUA_LAMBDA_FS__
#define  __LIB_MADDSUA_LAMBDA_FS__

#include <string>
#include <vector>

namespace Lambda::FS {

	void writeFileSync(const std::string& filepath, const std::vector<uint8_t>& data);
	void writeTextFileSync(const std::string& filepath, const std::string& data);

	std::vector<uint8_t> readFileSync(const std::string& filepath);
	std::string readTextFileSync(const std::string& filepath);

	bool existsSync(const std::string& filepath);

};

#endif
