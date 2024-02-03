#include <cstdio>
#include <stdexcept>

#include "../../lambda.hpp"

int main(int argc, char const *argv[]) {

	for (size_t i = 0; i < 25; i++) {
		auto errorMessage = Lambda::Errors::formatMessage(i);
		if (!errorMessage.size()) throw std::runtime_error("empty error message");

		for (auto c : errorMessage) {
			if (c < 32 || c > 125) {
				puts(errorMessage.c_str());
				throw std::runtime_error("invalid text string");
			}
		}
	}

	puts("Error codes ok");
	
	return 0;
}
