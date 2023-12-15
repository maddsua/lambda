#include <iostream>
#include <vector>

#include "../lambda.hpp"

using namespace Lambda;

int main(int argc, char const *argv[]) {

	std::vector<std::string> stringsToTrim = {
		"",
		" ",
		" aa",
		"aa ",
		" aa ",
		"  aa  ",
	};

	for (const auto& item : stringsToTrim) {
		std::cout << '|' << Strings::trim(item) << "|\n";
	}
	
	return 0;
}
