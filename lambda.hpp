/*
	This file is not the entry point!
	It just holds some stuff that is commonly user throughout the library.
*/

#ifndef __LIB_MADDSUA_LAMBDA__
#define __LIB_MADDSUA_LAMBDA__

#include <exception>
#include <stdexcept>
#include <string>

#define LAMBDAVERSION		"1.0.0"

#ifdef _WIN32
	#define LAMBDA_OS_API_ERRCODE_PREFIX ". WINAPI error code: "
#else
	#define LAMBDA_OS_API_ERRCODE_PREFIX ". OS error code: "
#endif

namespace Lambda {

	class Exception : public std::exception {
		private:
			std::string msg;
			int64_t code = 0;

		public:
			Exception(const std::string& message) : msg(message) {};
			Exception(const std::string& message, int64_t errorCode) : msg(message), code(errorCode) {};
			const char* what() const noexcept override {
				return (this->code ? (this->msg + LAMBDA_OS_API_ERRCODE_PREFIX + std::to_string(this->code)) : this->msg).c_str();
			}
	};

	class Error {
		public:
			Error() {};
			Error(const std::string& message) {
				this->what = message;
				isError = true;
			};
			Error(const std::string& message, const int64_t errorCode) {
				this->what = message;
				isError = true;
				code = errorCode;
			};

		bool isError = false;
		int64_t code = 0;
		std::string what;
	};
};

#endif
