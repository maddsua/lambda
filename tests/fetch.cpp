#include <iostream>
#include "../network/network.hpp"

int main(int argc, char const *argv[]) {

	auto response = Lambda::Network::fetch("http://google.com");

	std::cout << response.statusCode() << std::endl;
	std::cout << std::string(response.body.begin(), response.body.end()) << std::endl;

	/* code */
	return 0;
}
