#include <iostream>
#include "../core/core.hpp"

int main(int argc, char const *argv[]) {

	auto source = "https://user:password@example.com:443/document?search=query#fragment";

	auto url = HTTP::URL(source);

	std::cout << "original url: " << source << std::endl;

	std::cout << "hash: " << url.hash << std::endl;
	std::cout << "search: " << url.searchParams.stringify() << std::endl;
	std::cout << "protocol: " << url.protocol << std::endl;
	std::cout << "host: " << url.host << std::endl;
	std::cout << "hostname: " << url.hostname << std::endl;
	std::cout << "port: " << url.port << std::endl;
	std::cout << "pathname: " << url.pathname << std::endl;
	std::cout << "username: " << url.username << std::endl;
	std::cout << "password: " << url.password << std::endl;

	std::cout << "reassembled url: " << url.href() << std::endl;

	return 0;
}
