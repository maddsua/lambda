#include "../storage/kv.hpp"
#include <iostream>

using namespace Lambda;

int main() {

	puts("KV storage test\n");

	auto kv = Storage::KV();

	kv.set("user_0_name", "johndoe");
	kv.set("user_0_token", "xxxxx_token_content_xxxx");
	kv.move("user_0_token", "user_0_sessionid");
	kv.set("user_0_lastonline", "1688427615");
	kv.set("user_0_lastip", "127.0.0.1");


	puts("Exporting to json...");
	auto exportresult = kv.exportJSON("tests/data/kv_export.json");
	if (exportresult.isError()) throw std::runtime_error(exportresult.what());
	puts("Ok\n");

	puts("Restoring from json...");
	auto kv2 = Storage::KV();
	auto importresult = kv2.importJSON("tests/data/kv_export.json");
	if (importresult.isError()) throw std::runtime_error(importresult.what());
	puts("Ok\n");

	if (kv.get("user_0_name").value != "johndoe")
		throw std::runtime_error("Failed to retrieve 'user_0_name' record");

	if (kv.get("user_0_sessionid").value != "xxxxx_token_content_xxxx")
		throw std::runtime_error("Failed to retrieve 'user_0_name' record");

	if (kv.get("user_0_lastonline").value != "1688427615")
		throw std::runtime_error("Failed to retrieve 'user_0_name' record");

	if (kv.get("user_0_lastip").value != "127.0.0.1")
		throw std::runtime_error("Failed to retrieve 'user_0_name' record");


	return 0;
}
