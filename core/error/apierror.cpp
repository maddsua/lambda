#include "./error.hpp"

#ifdef _WIN32
	#include <windows.h>
	#define getAPIError() (GetLastError())
#else
	#include <cerrno>
	#define getAPIError() errno
#endif

using namespace Lambda;

static const size_t assumeMaxApiErrorMessageLength = 64;

int32_t Errors::getApiErrorCode() noexcept {

	return (
		#ifdef _WIN32
			GetLastError()
		#else
			errno
		#endif
	);
}

std::string Errors::formatErrorMessage(int32_t errorCode) noexcept {

	#ifdef _WIN32

		char* tempMessage = nullptr;
		auto formatResult = FormatMessageA(
			FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_MAX_WIDTH_MASK,
			NULL, errorCode, 0, (LPSTR)&tempMessage, 0, NULL);
		if (!formatResult) {
			return "Windows API error " + std::to_string(errorCode);
		}

		auto message = std::string(tempMessage);
		LocalFree(tempMessage);
		return message;

	#else

		#ifdef sys_errlist
			return (errorCode < sys_nerr) ? 
				sys_errlist[errorCode] :
				("OS error " + std::to_string(errorCode));
		#else
			char tempBuff[128];
			tempBuff[sizeof(tempBuff) - 1] = 0;
			if (strerror_s(tempBuff, sizeof(tempBuff) - 1, errorCode)) {
				return "OS error " + std::to_string(errorCode);
			} else return tempBuff;
		#endif

	#endif
}

APIError::APIError() {
	this->m_code = Errors::getApiErrorCode();
	this->m_text = Errors::formatErrorMessage(this->m_code);
}

APIError::APIError(const std::string& commentText) {
	this->m_code = Errors::getApiErrorCode();
	this->m_text = commentText + ' ' + Errors::formatErrorMessage(this->m_code);
}

APIError::APIError(int32_t code) {
	this->m_code = code;
	this->m_text = Errors::formatErrorMessage(code);
}

APIError::APIError(const APIError& other) {
	this->m_code = other.m_code;
	this->m_text = other.m_text;
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
