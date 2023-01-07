#include <iostream>
#include <string>
#include <regex>

#include <nlohmann/json.hpp>
using JSON = nlohmann::json;

#include "include/maddsua/lambda.hpp"
#include "include/maddsua/compress.hpp"
#include "include/maddsua/fs.hpp"


maddsua::lambdaResponse requesthandeler(maddsua::lambdaEvent event) {


	//	api calls, like real functions in AWS Lambda
	if (maddsua::startsWith(event.path, "/api")) {

		JSON data = {
			{"success", true},
			{"api-response", "succeded"},
			{"api-data", "test data"}
		};

		if (maddsua::searchQueryFind("user", &event.searchQuery) == "maddsua") {
			data["secret-message"] = "Buy some milk this time, come on Daniel =)";
		}
		
		return {
			200,
			{
				{ "content-type", maddsua::findMimeType("json") }
			},
			data.dump()
		};
	}

	//	fileserver part
	if (event.path[event.path.size() - 1] == '/') event.path += "index.html";
	event.path = std::regex_replace(("demo/" + event.path), std::regex("/+"), "/");

	std::string filecontents;

	if (!maddsua::readBinary(event.path, &filecontents)) {
		return { 404, {}, "File not found"};
	}

	auto fileext = event.path.find_last_of('.');

	return { 200, {
		{ "Content-Type", maddsua::findMimeType((fileext + 1) < event.path.size() ? event.path.substr(fileext + 1) : "bin")}
	}, filecontents};

}

int main(int argc, char** argv) {

	auto server = maddsua::lambda();
	auto startresult = server.init("27015", &requesthandeler);

	printf("Server: %s\r\n", startresult.cause.c_str());

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