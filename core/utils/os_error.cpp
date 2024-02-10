#include <string.h>
#include "./utils.hpp"

#ifdef _WIN32
	#include <windows.h>
#else
	#include <cerrno>
#endif

using namespace Lambda;

OS_Error::OS_Error() {
	this->m_code = (
		#ifdef _WIN32
			GetLastError()
		#else
			errno
		#endif
	);
}

OS_Error::OS_Error(int32_t code) : m_code(code) {}

std::string OS_Error::toString() const noexcept {
	#ifdef _WIN32

		char* tempMessage = nullptr;
		auto formatResult = FormatMessageA(
			FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_MAX_WIDTH_MASK,
			NULL, this->m_code, 0, (LPSTR)&tempMessage, 0, NULL);
		if (!formatResult) return "os error " + std::to_string(this->m_code);

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

int32_t OS_Error::code() const noexcept {
	this->m_code;
}
