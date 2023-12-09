#include <iostream>
#include <unordered_map>

#include "../extra/storage.hpp"

using namespace Storage;

int main(int argc, char const *argv[]) {

	LocalStorage localStorage;

	std::cout << "Get from local 'test_prop_1': " << localStorage.getItem("test_prop_1") << "\n";
	std::cout << "Get from local 'not_to_remove': " << localStorage.getItem("not_to_remove") << "\n";

	localStorage.setItem("test_prop_1", "quick brown horse... whatever");
	localStorage.setItem("test_prop_2", "very long string lol no it's not");
	localStorage.setItem("numbers woohooo", "100500");
	localStorage.setItem("478", "775");
	localStorage.setItem("to_remove", "this record will be removed");
	localStorage.setItem("not_to_remove", "this record will be not removed");

	localStorage.removeItem("to_remove");
	localStorage.removeItem("undefined");

	//localStorage.rebuild();

	return 0;
}