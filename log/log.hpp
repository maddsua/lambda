#ifndef __LIB_MADDSUA_LAMBDA_LOG__
#define __LIB_MADDSUA_LAMBDA_LOG__

#include <string>

namespace Lambda {

	namespace Log {

		struct Overload {
			std::string value;

			Overload() = default;

			Overload(const char* val) : value(val) {}
			Overload(const std::string& val) : value(val) {}
			Overload(bool val) : value(val ? "true" : "false") {}
			Overload(char val) : value(std::string(1, val)) {}
			Overload(unsigned char val) : value(std::to_string(val)) {}
			Overload(short val) : value(std::to_string(val)) {}
			Overload(unsigned short val) : value(std::to_string(val)) {}
			Overload(int val) : value(std::to_string(val)) {}
			Overload(unsigned int val) : value(std::to_string(val)) {}
			Overload(float val) : value(std::to_string(val)) {}
			Overload(double val) : value(std::to_string(val)) {}
			Overload(long val) : value(std::to_string(val)) {}
			Overload(unsigned long val) : value(std::to_string(val)) {}
			Overload(long long val) : value(std::to_string(val)) {}
			Overload(unsigned long long val) : value(std::to_string(val)) {}
			Overload(long double val) : value(std::to_string(val)) {}
		};

		void log(const std::string& format);
		void log(const std::string& format, std::initializer_list<Overload> params);

		void err(const std::string& message);
		void err(const std::string& format, std::initializer_list<Overload> params);
	};
};

#endif
