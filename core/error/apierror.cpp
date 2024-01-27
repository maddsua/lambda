#include "./error.hpp"

#ifdef _WIN32
	#include <errhandlingapi.h>
	#include <winbase.h>
	#define getAPIError() (GetLastError())
#else
	#include <cerrno>
	#define getAPIError() errno
#endif

using namespace Lambda;

static const size_t assumeMaxApiErrorMessageLength = 64;

int32_t ErrorHandling::getApiErrorCode() noexcept {
	return (
		#ifdef _WIN32
			GetLastError()
		#else
			errno
		#endif
	);
}

std::string ErrorHandling::formatErrorMessage(int32_t errorCode) noexcept {

	std::string message;

	#ifdef _WIN32

		char* tempMessage = nullptr;
		auto formatResult = FormatMessageA(
			FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
			NULL, errorCode, 0, (LPSTR)&tempMessage, 0, NULL);
		if (formatResult) {
			message = tempMessage;
			LocalFree(tempMessage);
		} else {
			message = "Windows API error " + std::to_string(errorCode);
		}

	#else

		#ifdef sys_errlist
			message = (errorCode < sys_nerr) ? 
				sys_errlist[errorCode] :
				("OS error " + std::to_string(errorCode));
		#else
			char tempBuff[128];
			tempBuff[sizeof(tempBuff) - 1] = 0;
			if (strerror_s(tempBuff, sizeof(tempBuff) - 1, errorCode)) {
				message = "OS error " + std::to_string(errorCode);
			} else message = tempBuff;
		#endif

	#endif

	return message;
}

APIError::APIError() {
	this->m_code = ErrorHandling::getApiErrorCode();
	this->m_text = ErrorHandling::formatErrorMessage(this->m_code);
}

const char* APIError::what() const noexcept {
	return this->m_text.c_str();
}

const std::string& APIError::message() const noexcept {
	return this->m_text;
}

int32_t APIError::code() const noexcept {
	return this->m_code;
}
