#include "./network.hpp"
#include "./sysnetw.hpp"

using namespace Lambda;
using namespace Lambda::Network;

NetworkError::NetworkError(const std::string& message) : Error(message) {
	this->m_code = GetOSErrorCode();
	this->m_text += " (os error " + std::to_string(this->m_code) + ')';
}

NetworkError::NetworkError(const std::string& message, int32_t errorCode) : Error(message) {
	this->m_code = errorCode;
	this->m_text += " (os error " + std::to_string(errorCode) + ')';
}

const int32_t NetworkError::osError() const noexcept {
	return this->m_code;
}
