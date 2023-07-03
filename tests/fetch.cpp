#include <iostream>
#include "../fetch/fetch.hpp"

int main(int argc, char const *argv[]) {

	auto response = Lambda::Fetch::fetch("http://google.com");

	std::cout << response.statusCode() << std::endl;
	std::cout << std::string(response.body.begin(), response.body.end()) << std::endl;

	return 0;
}
