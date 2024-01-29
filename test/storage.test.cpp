#include <iostream>
#include <unordered_map>

#include "../lambda.hpp"

using namespace Lambda::Storage;

int main(int argc, char const *argv[]) {

	auto testData = std::initializer_list<std::pair<std::string, std::string>>({
		{ "sample record", "sample value" },
		{ "test_prop_1", "quick brown horse... whatever" },
		{ "test_prop_2", "very long string lol no it's not" }
	});


	//	put some stuff into storage
	{
		LocalStorage localStorage("test/data/storage.db");

		for (const auto& item : testData) {
			localStorage.setItem(item.first, item.second);
		}

		puts("Write ok");
	}

	//	now get it out of there
	{
		LocalStorage localStorage("test/data/storage.db");

		for (const auto& item : testData) {
			auto value = localStorage.getItem(item.first);
			if (value != item.second) {
				throw std::runtime_error("data didn't match for record \"" + item.first + "\"\n\nExpected:\n" + item.second + "\n\nGot:\n" + value);
			}
		}

		puts("Data ok");
	}


	/*std::cout << "Get from local 'test_prop_1': " << localStorage.getItem("test_prop_1") << "\n";
	std::cout << "Get from local 'not_to_remove': " << localStorage.getItem("not_to_remove") << "\n";

	localStorage.setItem("test_prop_1", "quick brown horse... whatever");
	localStorage.setItem("test_prop_2", "very long string lol no it's not");
	localStorage.setItem("numbers woohooo", "100500");
	localStorage.setItem("478", "775");
	localStorage.setItem("to_remove", "this record will be removed");
	localStorage.setItem("not_to_remove", "this record will not be removed");

	localStorage.removeItem("to_remove");
	localStorage.removeItem("undefined");*/

	return 0;
}
