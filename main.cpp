#include <iostream>
#include <string>

#include "include/maddsua/lambda.hpp"
#include "include/maddsua/bufferCompress.hpp"

maddsuaHTTP::lambdaResponse requesthandeler(maddsuaHTTP::lambdaEvent event) {

	std::string body = "<h1>hello darkness my old friend</h1>";
		body += "Your user agent is: " + maddsuaHTTP::findHeader("User-Agent", &event.headers);

	if (maddsuaHTTP::findSearchQuery("user", &event.searchQuery) == "maddsua") {
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

/*	auto server = maddsuaHTTP::lambda();
	auto startresult = server.init("27015", &requesthandeler);

	printf("%s\r\n", startresult.cause.c_str());

	if (!startresult.success) return 1;

	puts("Waiting for connections at http://localhost:27015/");

	auto googled = maddsuaHTTP::fetch("google.com", "GET", {}, "");

	if (googled.errors.size()) puts(googled.errors.c_str());
	puts(std::to_string(googled.statusCode).c_str());
	puts(googled.body.c_str());

	while (true) {
		//	just chill while server is working
		Sleep(1000);
	}*/

	std::string textData = "1: this is a sample text string / 2: this is a sample text string / 3: this is a sample text string";

	auto uncompressed = std::vector<uint8_t>(textData.begin(), textData.end());

	std::vector<uint8_t> compressed;

	auto result = maddsuaCompress::compressVector(&uncompressed, &compressed, true);

	std::cout << "compression result: " << result << std::endl;
	std::cout << "Raw: " << textData.size() << " / compressed: " << compressed.size() << std::endl;

	std::vector<uint8_t> restored;

	auto result2 = maddsuaCompress::decompressVector(&compressed, &restored);

	std::cout << "Restored data: " << std::string(restored.begin(), restored.end()) << std::endl;

	return 0;
}