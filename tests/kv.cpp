#include "../storage/kv.hpp"
#include <iostream>

using namespace Lambda;

int main() {

	std::cout << "\r\n--- KV storage test begin --- \r\n";

	auto kv = Storage::KV();

	kv.set("user_0_name", "johndoe");
	kv.set("user_0_token", "xxxxx_token_content_xxxx");
	kv.move("user_0_token", "user_0_sessionid");
	kv.set("user_0_lastonline", "1688427615");
	kv.set("user_0_lastip", "127.0.0.1");


	std::cout << "String expected: " << kv.get("user_0_name").value << std::endl;
	std::cout << "Empty string expected: " << kv.get("user_0_token").value << std::endl;
	std::cout << "String expected: "<< kv.get("user_0_sessionid").value << std::endl;

	std::cout << "\r\n--- Exporting to json --- \r\n";
	auto exportresult = kv.exportJSON("tests/data/kv_export.json");
	if (exportresult.isError()) std::cout << "Error: " << exportresult.what() << std::endl;
		else std::cout << "Ok" << std::endl;

	std::cout << "\r\n--- Restoring from json --- \r\n";
	auto kv2 = Storage::KV();
	auto importresult = kv2.importJSON("tests/data/kv_export.json");
	if (importresult.isError()) std::cout << "Error: " << exportresult.what() << std::endl;
		else std::cout << "Ok" << std::endl;

	std::cout << "\r\n--- List of all records: --- \r\n";

	auto listing = kv2.entries();
	for (auto item : listing) {
		std::cout << item.key << " : " << item.value << std::endl;
	}

	return 0;
}
