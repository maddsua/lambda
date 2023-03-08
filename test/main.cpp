#include <stdio.h>
#include <string>
#include <iostream>

#include "../include/lambda.hpp"

maddsuahttp::lambdaResponse requesthandeler(maddsuahttp::lambdaEvent event) {

	std::string body = "<h1>hello darkness my old friend</h1>";
		body += "Your user agent is: " + maddsuahttp::findHeader("User-Agent", &event.headers);

	return {
		200,
		{
			{"test", "maddsua"}
		},
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
		//	just chill while server is working
		Sleep(1000);
	}

	return 0;
}