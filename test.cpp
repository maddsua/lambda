#include <iostream>
#include "./core/core.hpp"

int main(int argc, char const *argv[]) {

	auto url = new HTTP::URL("https://example.com:443/doc?v=1#fragment");

	std::cout << "hash: " << url->hash << std::endl;
	std::cout << "search: " << url->searchParams.stringify() << std::endl;
	std::cout << "protocol: " << url->protocol << std::endl;
	std::cout << "host: " << url->host << std::endl;
	std::cout << "hostname: " << url->hostname << std::endl;
	std::cout << "port: " << url->port << std::endl;
	std::cout << "pathname: " << url->pathname << std::endl;

	/* code */
	return 0;
}
