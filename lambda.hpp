/*
	This file is not the entry point!
	It just holds some stuff that is commonly user throughout the library.
*/

#ifndef __LIB_MADDSUA_LAMBDA__
#define __LIB_MADDSUA_LAMBDA__

#include <exception>
#include <stdexcept>
#include <string>

#define LAMBDA_PRODUCT			"maddsua/lambda"
#define LAMBDA_VERSION			"1.0.0"
#define LAMBDA_USERAGENT		LAMBDA_PRODUCT "/" LAMBDA_VERSION
#define LAMBDA_FETCH_ENCODINGS	"br, gzip, deflate"

//#define LAMBDADEBUG_WS

#ifdef _WIN32
	#define LAMBDA_OS_API_ERRCODE_PREFIX ". WINAPI error code: "
#else
	#define LAMBDA_OS_API_ERRCODE_PREFIX ". OS error code: "
#endif

namespace Lambda {

	class Error : public std::exception {
		private:
			std::string msg;
			int64_t code = 0;
			bool error = false;

			void formatMessage() {
				msg = (this->code ? (this->msg + LAMBDA_OS_API_ERRCODE_PREFIX + std::to_string(this->code)) : this->msg);
			}

		public:
			Error() {};
			Error(const std::string& message) : msg(message), error(true) {};
			Error(const std::string& message, int64_t errorCode) : msg(message), code(errorCode), error(true) {
				formatMessage();
			}
			const char* what() const noexcept override {
				return msg.c_str();
			}
			bool isError() { return this->error; };
			int64_t errorCode() { return this->code; };
	};

};

#endif
