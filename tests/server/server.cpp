#include "../../server/server.hpp"
#include <windows.h>

int main() {

	auto server = Lambda::Server();

	while (true) {
		if (!server.hasNewLogs()) {
			Sleep(10);
			continue;
		}
		puts(HTTP::stringJoin(server.logsText(), "\n").c_str());
	}
	
	return 0;
}