#include <iostream>
#include <string>

#include "../include/maddsua/lambda.hpp"
#include "../include/maddsua/compress.hpp"
#include "../include/maddsua/fs.hpp"

maddsua::lambdaResponse requesthandeler(maddsua::lambdaEvent event) {

	std::string body = "<h1>hello darkness my old friend</h1>";
		body += "Your user agent is: " + maddsua::headerFind("User-Agent", &event.headers);

	if (maddsua::searchQueryFind("ask", &event.searchQuery) == "google") {

		body = "<h2>Good night, my Dark Lord</h2>\r\n";

		//	connect to google.com
		{
			auto googeResp = maddsua::fetch("google.com", "GET", {}, "");
			printf("Connecting to google.com... %i %s", googeResp.statusCode, googeResp.statusText.c_str());
				if (googeResp.errors.size()) puts(googeResp.errors.c_str());
			body += "<p>This is what google says: Page " + googeResp.statusText + "</p>";
		}
	}

	std::string htmlpage;
	if (maddsua::readBinary("index.html", &htmlpage)) body = htmlpage;
		else body += "\r\nHave tried to load index.html, but didn't fount it =((\r\n";

	return {
		200,
		{
			{"x-test", "maddsua"}
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

	while (true) {

		for (auto log : server.logs()) {
			std::cout << log << std::endl;
		}
		
		//	just chill while server is running
		Sleep(1000);
	}

	return 0;
}