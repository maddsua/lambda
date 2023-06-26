#ifndef __LAMBDA_EXCEPTION__
#define __LAMBDA_EXCEPTION__

#include <exception>
#include <string>

namespace Lambda {

	class exception : public std::exception {
		public:
			exception(const std::string& message) : _msg(message) {}
			virtual const char* what() const noexcept override {
				return _msg.c_str();
			}
		private:
			std::string _msg;
	};

};


#endif