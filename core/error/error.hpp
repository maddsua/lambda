#ifndef __LIB_MADDSUA_LAMBDA_CORE_ERROR_HANDLING__
#define __LIB_MADDSUA_LAMBDA_CORE_ERROR_HANDLING__

#include <stdexcept>
#include <string>

namespace Lambda {

	typedef std::exception Error;

	namespace ErrorHandling {
		int32_t getApiErrorCode() noexcept;
		std::string formatErrorMessage(int32_t errorCode) noexcept;
	};

	class APIError : public Error {
		private:
			int32_t m_code = 0;
			std::string m_text;

		public:
			APIError() = default;
			const char* what() const noexcept override;
			const std::string& message() const noexcept;
			int32_t code() const noexcept;
	};

};

#endif
