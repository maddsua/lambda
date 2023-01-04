#include <iostream>
#include <string>

#include "../include/maddsua/lambda.hpp"

maddsuahttp::lambdaResponse requesthandeler(maddsuahttp::lambdaEvent event) {

	std::string body = "<h1>hello darkness my old friend</h1>";
		body += "Your user agent is: " + maddsuahttp::findHeader("User-Agent", &event.headers);

	if (maddsuahttp::findSearchQuery("user", &event.searchQuery) == "maddsua") {
		body = "Good night, my Dark Lord";
	}
	
	return {
		200,
		{
			{"test", "maddsua"}
		},
		body
	};
}

int main(int argc, char** argv) {

	/*auto server = maddsuahttp::lambda();
	auto startresult = server.init("27015", &requesthandeler);

	printf("%s\r\n", startresult.cause.c_str());

	if (!startresult.success) return 1;

	puts("Waiting for connections at http://localhost:27015/");

	auto googled = maddsuahttp::fetch("google.com", "GET", {}, "");

	if (googled.errors.size()) puts(googled.errors.c_str());
	puts(std::to_string(googled.statusCode).c_str());
	puts(googled.body.c_str());

	while (true) {
		//	just chill while server is working
		Sleep(1000);
	}*/

    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET){
		auto code = GetLastError();
		std::cout << code << std::endl;
    }

    closesocket(s);

	return 0;
}