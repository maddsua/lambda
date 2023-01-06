#include <iostream>
#include <string>

#include "include/maddsua/lambda.hpp"
#include "include/maddsua/compress.hpp"
#include "include/maddsua/fs.hpp"

maddsua::lambdaResponse requesthandeler(maddsua::lambdaEvent event) {

	std::string body = "<h1>hello darkness my old friend</h1>";
		body += "Your user agent is: " + maddsua::headerFind("User-Agent", &event.headers);

	if (maddsua::searchQueryFind("user", &event.searchQuery) == "maddsua") {
		body = "Good night, my Dark Lord";
	}

	body += "<br><br>Some text to test network mechanistms";

	body += "<br><br>Even more text content here to test network compression functionality";
	
	return {
		200,
		{
			{ "test", "maddsua" }
		},
		body
	};
}

int main(int argc, char** argv) {

	auto server = maddsua::lambda();
	auto startresult = server.init("27015", &requesthandeler);

	printf("%s\r\n", startresult.cause.c_str());

	if (!startresult.success) return 1;

	puts("Waiting for connections at http://localhost:27015/");

	//	connect to google.com
	/*{
		auto googeResp = maddsua::fetch("www.google.com", "GET", {}, "");

		printf("Connecting to google.com... %i %s", googeResp.statusCode, googeResp.statusText);
		if (googeResp.errors.size()) puts(googeResp.errors.c_str());
		puts(googeResp.body.c_str());

		for (auto header : googeResp.headers) {
			std::cout << header.name << " " << header.value << std::endl;
		}


		std::cout << "Writing to googlecom.bin result: " << maddsua::writeBinary("googlecom.bin", &googeResp.body) << std::endl;
	}*/

	while (true) {

		for (auto log : server.logs()) {
			std::cout << log << std::endl;
		}
		//	just chill while server is running
		Sleep(1000);
	}

	return 0;
}