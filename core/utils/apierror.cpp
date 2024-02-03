#include <string.h>
#include "./utils.hpp"

#ifdef _WIN32
	#include <windows.h>
#else
	#include <cerrno>
#endif

using namespace Lambda;

int32_t Errors::getApiError() noexcept {
	return (
		#ifdef _WIN32
			GetLastError()
		#else
			errno
		#endif
	);
}

std::string Errors::formatMessage(int32_t errorCode) noexcept {

	#ifdef _WIN32

		char* tempMessage = nullptr;
		auto formatResult = FormatMessageA(
			FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_MAX_WIDTH_MASK,
			NULL, errorCode, 0, (LPSTR)&tempMessage, 0, NULL);
		if (!formatResult) return "Windows API error " + std::to_string(errorCode);

		auto message = std::string(tempMessage);
		LocalFree(tempMessage);
		return message;

	#else

		#ifdef sys_errlist

			if (errorCode < sys_nerr)
				return sys_errlist[errorCode];
			return "OS error " + std::to_string(errorCode);

		#else

			char tempBuff[128];
			char* messagePtr = strerror_s(tempBuff, sizeof(tempBuff), errorCode);
			if (messagePtr != nullptr) return messagePtr;
			return "OS error " + std::to_string(errorCode);

		#endif

	#endif
}


APIError::APIError() {
	this->m_code = Errors::getApiError();
	this->m_text = Errors::formatMessage(this->m_code);
}

APIError::APIError(const std::string& commentText) {
	this->m_code = Errors::getApiError();
	this->m_text = commentText + " (" + Errors::formatMessage(this->m_code) + ")";
}

APIError::APIError(int32_t code) {
	this->m_code = code;
	this->m_text = Errors::formatMessage(code);
}

APIError::APIError(int32_t code, const std::string& commentText) {
	this->m_code = code;
	this->m_text = commentText + " (" + Errors::formatMessage(code) + ")";
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
