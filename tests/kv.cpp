#include "../storage/kvstore.hpp"
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

	std::cout << "Shoult output a string: " << kv.get("user_0_name").value << std::endl;
	std::cout << "Shoult output nothing: " << kv.get("user_0_token").value << std::endl;
	std::cout << "Shoult output a string: "<< kv.get("user_0_sessionid").value << std::endl;

	std::cout << "\r\n--- Writing to \"kvstore.db\" --- \r\n";
	kv.exportStore("kvstore.db");
	std::cout << "Ok" << std::endl;

	std::cout << "\r\n--- Restoring from \"kvstore.db\" --- \r\n";
	auto kv2 = Storage::KV();
	kv2.importStore("kvstore.db");
	std::cout << std::endl;

	std::cout << "Shoult output a string: "<< kv.get("user_0_name").value << std::endl;

	return 0;
}
