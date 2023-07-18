#include <iostream>
#include "../client/client.hpp"

int main(int argc, char const *argv[]) {

	auto response = Lambda::Client::fetch(":8888/");

	std::cout << response.statusCode() << std::endl;
	std::cout << std::string(response.body.begin(), response.body.end()) << std::endl;

	return 0;
}
