#include "./network.hpp"
#include "./sysnetw.hpp"

using namespace Lambda;


#ifdef _WIN32

	bool Lambda::Network::wsaWakeUp() {

		static bool wsaInitCalled = false;
		if (wsaInitCalled) return false;
		wsaInitCalled = true;

		WSADATA initdata;
		if (WSAStartup(MAKEWORD(2,2), &initdata) != 0)
			throw std::runtime_error("WSA initialization failed: windows API error " + std::to_string(getAPIError()));

		return true;
	}

#endif


void Network::setConnectionTimeouts(SOCKET hSocket, uint32_t timeoutsMs) {
	if (setsockopt(hSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeoutsMs, sizeof(timeoutsMs)))
		throw std::runtime_error("failed to set socket RX timeout: code " + std::to_string(getAPIError()));
	if (setsockopt(hSocket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeoutsMs, sizeof(timeoutsMs)))
		throw std::runtime_error("failed to set socket TX timeout: code " + std::to_string(getAPIError()));
}
