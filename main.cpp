#include <iostream>
#include <string>

#include "include/maddsua/lambda.hpp"
#include "include/maddsua/compress.hpp"
#include "include/maddsua/fs.hpp"

maddsua::lambdaResponse requesthandeler(maddsua::lambdaEvent event) {

	std::string body = "<h1>hello darkness my old friend</h1>";
		body += "Your user agent is: " + maddsua::findHeader("User-Agent", &event.headers);

	if (maddsua::findSearchQuery("user", &event.searchQuery) == "maddsua") {
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

	auto server = maddsua::lambda();
	auto startresult = server.init("27015", &requesthandeler);

	printf("%s\r\n", startresult.cause.c_str());

	if (!startresult.success) return 1;

	puts("Waiting for connections at http://localhost:27015/");

	//	connect to google.com
	/*{
		auto googeResp = maddsua::fetch("google.com", "GET", {}, "");

		printf("Connecting to google.com... %i %s", googeResp.statusCode, googeResp.statusText);
		if (googeResp.errors.size()) puts(googeResp.errors.c_str());
		puts(googeResp.body.c_str());

		auto uncompressed = std::vector<uint8_t>(googeResp.body.begin(), googeResp.body.end());
		std::vector<uint8_t> compressed;
		auto result = maddsua::gzCompress(&uncompressed, &compressed, true);
		std::cout << "Compression result: " << result << std::endl;
		std::cout << "Raw: " << uncompressed.size() << " / compressed: " << compressed.size() << std::endl;

		std::cout << "Writing to googlecom.html.gz result: " << maddsua::writeBinary("googlecom.html.gz", &compressed) << std::endl;
	}*/

	while (true) {

		for (auto log : server.logs()) {
			puts(log.text.c_str());
		}
		//	just chill while server is working
		Sleep(1000);
	}

	return 0;
}