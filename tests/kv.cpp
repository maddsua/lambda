#include "../storage/kv.hpp"
#include <iostream>

using namespace Lambda;

int main() {

	std::cout << "\r\n--- KV storage test begin --- \r\n";

	auto kv = Storage::KV();

	/*kv.set("user_0_name", "johndoe");
	kv.set("user_0_token", "xxxxx_token_content_xxxx");
	kv.move("user_0_token", "user_0_sessionid");
	kv.set("user_0_lastonline", "1688427615");
	kv.set("user_0_lastip", "127.0.0.1");*/

	//	add a ton of records
	for (size_t i = 0; i < 100000; i++) {
		auto user = "user_" + std::to_string(i) + "_";
		kv.set(user + "name", "johndoe");
		kv.set(user + "token", "xxxxx_token_content_xxxx");
		kv.move(user + "token", user + "sessionid");
		kv.set(user + "lastonline", "1688427615");
		kv.set(user + "lastip", "127.0.0.1");
	}	

	std::cout << "String expected: " << kv.get("user_0_name").value << std::endl;
	std::cout << "Empty string expected: " << kv.get("user_0_token").value << std::endl;
	std::cout << "String expected: "<< kv.get("user_0_sessionid").value << std::endl;

	std::cout << "\r\n--- Writing to \"kvstore.db\" --- \r\n";
	kv.exportStore("kvstore.db", Storage::KVSTORE_COMPRESS_BR);
	std::cout << "Ok" << std::endl;

	std::cout << "\r\n--- Restoring from \"kvstore.db\" --- \r\n";
	auto kv2 = Storage::KV();
	kv2.importStore("kvstore.db");
	std::cout << std::endl;

	std::cout << "String expected: "<< kv2.get("user_0_name").value << std::endl;

	/*std::cout << "\r\n--- List of all records: --- \r\n";

	auto listing = kv2.entries();
	for (auto item : listing) {
		std::cout << item.key << " : " << item.value << std::endl;
	}*/

	return 0;
}
