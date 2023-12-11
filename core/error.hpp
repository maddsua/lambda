#ifndef __LIB_MADDSUA_LAMBDA_ERROR__
#define __LIB_MADDSUA_LAMBDA_ERROR__

#include <exception>
#include <stdexcept>
#include <string>

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
