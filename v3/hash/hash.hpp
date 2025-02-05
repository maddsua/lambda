#ifndef __LAMBDA_HASH__
#define __LAMBDA_HASH__

#include <cstdint>
#include <vector>
#include <optional>
#include <array>

#include "../http/http.hpp"

namespace Lambda::Hash {

	namespace SHA1 {

		std::vector<uint8_t> text(const std::string& data);

	};

};

#endif
