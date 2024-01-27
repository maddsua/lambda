#include <cstdio>
#include "../core/error/error.hpp"

int main(int argc, char const *argv[]) {

	for (size_t i = 0; i < 25; i++) {
		auto errorMessage = Lambda::Errors::formatMessage(i);
		printf("Test error message: %s\n", errorMessage.c_str());
	}
	
	return 0;
}
