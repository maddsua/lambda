#include <stdio.h>
#include <string>

#include "include/lambda.hpp"

maddsuahttp::lambdaResponse requesthandeler(maddsuahttp::lambdaEvent event) {

	if (maddsuahttp::findSearchQuery("user", &event.searchQuery) == "maddsua")
		return {
			200,
			{
				{"test", "maddsua"}
			},
			"Good night, my Dark Lord"
		};

	std::string body = "<h1>hello darkness my old friend</h1>";
		body += "Your user agent is: " + maddsuahttp::findHeader("User-Agent", &event.headers);

	return {
		200,
		{},
		body
	};
}

int main(int argc, char** argv) {

	auto server = maddsuahttp::lambda();
	auto startresult = server.init("27015", &requesthandeler);

	printf("%s\r\n", startresult.cause.c_str());

	if (!startresult.success) return 1;

	puts("Waiting for connections at http://localhost:27015/");

	while (true) {

		if (server.logsAvail()) {
			auto logs = server.logs();
			for (auto entry : logs) {
				puts(entry.text.c_str());
			}
		}

		Sleep(250);
	}

	return 0;
}