#ifndef __LIB_MADDSUA_LAMBDA_CORE_ERROR_HANDLING__
#define __LIB_MADDSUA_LAMBDA_CORE_ERROR_HANDLING__

#include <stdexcept>
#include <string>

namespace Lambda {

	typedef std::exception Error;

	namespace Errors {
		int32_t getApiError() noexcept;
		std::string formatMessage(int32_t errorCode) noexcept;
	};

	/*class APIError : public Error {
		private:
			int32_t m_code = 0;
			std::string m_text;

		public:
			APIError();
			APIError(int32_t code);
			APIError(const std::string& commentText);
			APIError(const APIError& other);
			const char* what() const noexcept override;
			const std::string& message() const noexcept;
			int32_t code() const noexcept;
	};*/

};

#endif
