#include <iostream>
#include <stdexcept>

#include "../../lambda.hpp"

using namespace Lambda;

int main(int argc, char const *argv[]) {

	/*
		Check for serialization errors resulling in incorrect or garbage data
	*/
	static const std::string compareValid = "userid=test_user_0\nx_lambda=control; Secure; expires=23 Oct 2077 08:28:00 GMT";

	for (size_t i = 0; i < 100; i++) {

		auto newCookies = HTTP::Cookies();
		newCookies.set("userid", "test_user_0");
		newCookies.set("x_lambda", "control", {
			"Secure",
			{ "expires", "23 Oct 2077 08:28:00 GMT" }
		});

		const auto generated = Strings::join(newCookies.serialize(), "\n");

		if (generated != compareValid) {
			std::cout << "Cookie run # " << i << " produced invalid result:\n" << generated << "\n\n";
			throw std::runtime_error("Invalid serialization result");
		}
	}

	puts("Cookie serialization test ok");

	return 0;
}
