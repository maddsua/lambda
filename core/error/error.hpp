#ifndef __LIB_MADDSUA_LAMBDA_CORE_ERROR_HANDLING__
#define __LIB_MADDSUA_LAMBDA_CORE_ERROR_HANDLING__

#include <stdexcept>
#include <string>

namespace Lambda::Errors {
	int32_t getApiError() noexcept;
	std::string formatMessage(int32_t errorCode) noexcept;
};

#endif
