#include "../network.hpp"
#include "../sysnetw.hpp"

using namespace Lambda;

struct UniqueAddrInfo {

	addrinfo* info = nullptr;

	void get(const std::string& host, const std::string& port, addrinfo hints) {
		const auto getResult = getaddrinfo(host.c_str(), port.c_str(), &hints, &this->info);
		if (getResult) {
			throw Lambda::APIError("failed to resolve hostname " + host);
		}
	}

	~UniqueAddrInfo() {
		if (this->info == nullptr) return;
		freeaddrinfo(this->info);
	}
};
