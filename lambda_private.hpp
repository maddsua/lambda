/*
	This file is not the entry point!
	It just holds some stuff that is commonly user throughout the library.

	The http and websocket C++ library
	Or at least, it apperas to be
	2023 maddsua, https://github.com/maddsua

	This file is not the entrypoint.
	Include lambda.hpp instead
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
			bool error = false;

		public:
			/**
			 * Empty constructor. Means to error
			*/
			Error() {};
			/**
			 * Simple constructor, the same as std::exception
			*/
			Error(const std::string& message) : msg(message), error(true) {};

			/**
			 * Constructor with an error code
			*/
			Error(const std::string& message, int64_t errorCode) : msg(message), error(true) {
				if (!errorCode) return;
				msg += " [code: " + std::to_string(errorCode) + "]";
			}

			/**
			 * Constructor with another error(s) text included
			*/
			Error(const std::string& message, const std::exception& errorInstance) : msg(message), error(true) {
				msg += std::string(" << ") + errorInstance.what();
			}

			/**
			 * Same, but less fancy
			*/
			Error(const std::string& message, const Error& errorInstance) : msg(message), error(true) {
				if (!errorInstance.isError()) return;
				msg += std::string(" << ") + errorInstance.what();
			}

			/**
			 * Dump error text
			*/
			const char* what() const noexcept override {
				return msg.c_str();
			}

			/**
			 * True in an error state
			*/
			const bool isError() const noexcept { return this->error; };
	};

};

#endif
