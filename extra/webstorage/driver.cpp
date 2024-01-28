
#include "./interface.hpp"
#include "./driver.hpp"

using namespace Lambda::Storage;
using namespace Lambda::Storage::WebStorage;

KVDriver::KVDriver(const std::string& filename) {
	puts("creating driver");
}

KVDriver::~KVDriver() {
	puts("destructing driver");
}

void KVDriver::handleTransaction(const Transaction&) {
	puts("handling a transaction");
}
