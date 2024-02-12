#include <string.h>
#include "./utils.hpp"

#ifdef _WIN32
	#include <windows.h>
	#define GetOSErrorCode() (GetLastError())
#else
	#include <cerrno>
	#define GetOSErrorCode() (errno)
#endif

using namespace Lambda;

OS_Error Lambda::getOSError() noexcept {

	/*
		Error code casted to a i32, it's not like winapi features
		any errors with codes bigger than it can hold.
		Also unix uses it by default, so that's why.
	*/
	return OS_Error(GetOSErrorCode());
}

std::string OS_Error::message() const noexcept {
	#ifdef _WIN32

		char* tempMessage = nullptr;
		auto formatResult = FormatMessageA(
			FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_MAX_WIDTH_MASK | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, this->code, 0, (LPSTR)&tempMessage, 0, NULL);
		if (!formatResult) return "os error " + std::to_string(this->code);

		auto message = std::string(tempMessage);
		LocalFree(tempMessage);
		return message;

	#else

		#ifdef sys_errlist

			if (this->code < sys_nerr)
				return sys_errlist[this->code];
			return "os error " + std::to_string(this->code);

		#else

			char tempBuff[128];
			char* messagePtr = strerror_r(this->code, tempBuff, sizeof(tempBuff));
			if (messagePtr != nullptr) return messagePtr;
			return "os error " + std::to_string(this->code);

		#endif

	#endif
}
