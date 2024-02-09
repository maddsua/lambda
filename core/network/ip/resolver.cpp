#include "../network.hpp"
#include "../sysnetw.hpp"

using namespace Lambda;

struct UniqueAddrInfo {

	addrinfo* info = nullptr;

	void get(const std::string& host, const std::string& port, addrinfo hints) {
		if (getaddrinfo(host.c_str(), port.c_str(), &hints, &this->info)) {
			throw Lambda::APIError("failed to resolve hostname " + host);
		}
	}

	~UniqueAddrInfo() {
		if (this->info == nullptr) return;
		freeaddrinfo(this->info);
	}
};
