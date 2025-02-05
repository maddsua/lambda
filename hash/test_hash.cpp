#include <cstdio>
#include <stdexcept>
#include <functional>
#include <vector>

#include "./hash.hpp"
#include "../base64/base64.hpp"
#include "../utils/tests.hpp"

using namespace Lambda::Hash;
using namespace Lambda::Encoding;

//	JS test function:
//	const hash_text = async (text) => btoa(String.fromCharCode(...new Uint8Array(await crypto.subtle.digest("SHA-1", new TextEncoder().encode(text)))));

void test_hash_1() {

	std::string data = "a smol test string";

	auto hash = SHA1::text(data);

	string_assert("result", Base64::encode(hash), "tx1QJmyZAAYrdg2mXpCuT2eJ+GI=");
}

void test_hash_2() {

	std::string data = "websockets";

	auto hash = SHA1::text(data);

	string_assert("result", Base64::encode(hash), "AHXHHdl3pz/pLXP3STaSzq7OWvI=");
}

int main() {

	auto queue = TestQueue({
		{ "hash_1", test_hash_1 },
		{ "hash_2", test_hash_2 },
	});

	return exec_queue(queue);
}
