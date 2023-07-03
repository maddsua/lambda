#include "../storage/kvstore.hpp"
#include <iostream>

using namespace Lambda;

int main() {

	auto kv = Storage::KV();

	kv.set("redis", "cool");
	kv.set("upstash", "nice");
	kv.move("upstash", "redisflare");
	kv.set("username", "maddsua");
	kv.set("userid", "0");
	kv.set("usertoken", "rooooooot");

	std::cout << kv.get("redis").value << std::endl;
	std::cout << kv.get("upstash").value << std::endl;
	std::cout << kv.get("redisflare").value << std::endl;

	kv.exportStore("db.db");

	auto kv2 = Storage::KV();
	kv2.importStore("db.db");

	std::cout << kv2.get("username").value << std::endl;

	return 0;
}
