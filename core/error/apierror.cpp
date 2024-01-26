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

int32_t Lambda::getApiErrorCode() noexcept {
	return (
		#ifdef _WIN32
			GetLastError()
		#else
			errno
		#endif
	);
}

std::string Lambda::formaErrorMessage(int32_t errorCode) noexcept {

	std::string message;

	#ifdef _WIN32

		char* tempMessage = nullptr;
		auto formatResult = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, errorCode, 0, (LPSTR)&tempMessage, 0, NULL);
		if (formatResult) {
			message = tempMessage;
			LocalFree(tempMessage);
		} else {
			message = "Windows API error " + std::to_string(errorCode);
		}

	#else


		message = "ahh shit it's not implemented yet";
		
	#endif
 
	strerror_s();

	return message;
}

APIError::APIError() {
	this->m_code = getApiErrorCode();
	this->m_text = formaErrorMessage(this->m_code);
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
