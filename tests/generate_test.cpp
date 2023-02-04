#include <iostream>

#include "../include/maddsua/crypto.hpp"


int main(int argc, char** argv) {

	std::cout << "Let's create a UUID: " << maddsua::createUUID() << std::endl;

	std::cout << "\r\nAnd a few random numbers:\r\n";

	auto numbers = maddsua::randomSequence(100, 5);
	for (auto num : numbers) {
		std::cout << num << " ";
	}
	std::cout << std::endl;

	std::cout << "\r\nCreating a random password: " << maddsua::createPassword(16, true);
	

	return 0;
}
