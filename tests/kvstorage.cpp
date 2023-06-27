#include "../storage/storage.hpp"
#include <iostream>

using namespace Lambda;

int main() {

	auto kv = Storage::KV();

	kv.set("redis", "cool");
	kv.set("upstash", "nice");
	kv.move("upstash", "redisflare");

	std::cout << kv.get("redis").value << std::endl;
	std::cout << kv.get("upstash").value << std::endl;
	std::cout << kv.get("redisflare").value << std::endl;

	return 0;
}
