#include <iostream>
#include <stdexcept>

#include "../../lambda.hpp"

using namespace Lambda;

static const std::string compareData = "connection: upgrade\r\ncontent-length: 100\r\ncontent-encoding: br\r\n";

int main(int argc, char const *argv[]) {

	HTTP::Headers headers = {
		{ "ContenT-encoDinG", "br" },
		{ " content-length  \t", "\t 100  " },
		{ "connection", "upgrade" }
	};

	std::string headersSerialized;

	for (const auto& item : headers.entries()) {
		headersSerialized += item.first + ": " + item.second + "\r\n";
	}

	if (headersSerialized != compareData) {
		throw std::runtime_error("Serialized data mistmatch");
	}

	puts("Headers serialization test ok");

	return 0;
}
