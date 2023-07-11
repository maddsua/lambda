/*
 * This file is not the entry point!
 * It just holds some stuff that is commonly user throughout the library.
 * 
 * The http and websocket C++ library
 * Or at least, it apperas to be
 * 2023 maddsua, https://github.com/maddsua
 * 
 * This file is not the entrypoint.
 * Include lambda.hpp instead
*/

#ifndef __LIB_MADDSUA_LAMBDA_PRIVATE__
#define __LIB_MADDSUA_LAMBDA_PRIVATE__

#include "./lambda_version.hpp"

#include <exception>
#include <stdexcept>
#include <string>

#define LAMBDA_USERAGENT		LAMBDA_PRODUCT "/" LAMBDA_VERSION
#define LAMBDA_FETCH_ENCODINGS	"br, gzip, deflate"

//#define LAMBDADEBUG_WS

namespace Lambda {

	class Error : public std::exception {
		private:
			std::string msg;
			int64_t code = 0;
			bool error = false;

			void formatMessage() {
				msg = (this->code ? (this->msg + "Error code: " + std::to_string(this->code)) : this->msg);
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
