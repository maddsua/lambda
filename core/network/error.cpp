#include "./network.hpp"
#include "./sysnetw.hpp"

using namespace Lambda;
using namespace Lambda::Net;

NetworkError::NetworkError(const std::string& message) : Error(message) {
	this->m_code = GetOSErrorCode();
	if (this->m_code) this->m_text += " (os error " + std::to_string(this->m_code) + ')';
}

NetworkError::NetworkError(const std::string& message, int32_t errorCode) : Error(message) {
	this->m_code = errorCode;
	if (this->m_code) this->m_text += " (os error " + std::to_string(this->m_code) + ')';
}

const int32_t NetworkError::osError() const noexcept {
	return this->m_code;
}
