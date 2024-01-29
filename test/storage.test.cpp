#include <iostream>
#include <unordered_map>

#include "../lambda.hpp"

using namespace Lambda::Storage;

int main(int argc, char const *argv[]) {

	auto testData = std::initializer_list<std::pair<std::string, std::string>>({
		{ "sample record", "sample value" },
		{ "test_prop_1", "quick brown horse... whatever" },
		{ "test_prop_2", "very long string lol no it's not" },
		{ "to_remove", "this record will be removed" },
		{ "also_remove", "this record will not be removed" },
	});

	auto removeList = std::vector<std::string>({
		"to_remove",
		"also_remove"
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

		for (const auto& item : removeList) {
			localStorage.removeItem(item);
		}

		localStorage.removeItem("undefined");

		puts("Data ok");
	}

	//	test record removal
	{
		LocalStorage localStorage("test/data/storage.db");

		for (const auto& item : removeList) {
			if (localStorage.hasItem(item)) {
				throw std::runtime_error("record \"" + item + "\" was not removed");
			}
		}

		puts("Record removal ok");
	}

	return 0;
}
