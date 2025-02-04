#include <cstdio>
#include <stdexcept>
#include <functional>
#include <vector>

#include "./json.hpp"
#include "../utils/tests.hpp"

using namespace Lambda::JSON;

void test_stringify_1() {

	Array data = {
		8,
		3.14,
		150,
		9,
		Object({
			{"record", 77},
			{"record_2", 150000},
		})
	};

	auto serialized = stringify(data);

	string_assert("result", serialized, R"([8,3.14,150,9,{"record":77,"record_2":150000}])");
}

void test_stringify_2() {

	auto serialized = stringify(42);

	string_assert("result", serialized, R"(42)");
}

void test_stringify_3() {

	Object data = {
		{"nice", 69},
		{"weight", 150},
		{"command", "Look here"},
		{"bruh", true},
		{"points", Array {
			1,2,3,4,5,6,7
		}},
	};

	auto serialized = stringify(data);

	string_assert("result", serialized, R"({"bruh":true,"command":"Look here","nice":69,"points":[1,2,3,4,5,6,7],"weight":150})");
}

void test_stringify_4() {

	auto serialized = stringify(Array {13.75, 10.00, 0.0, -0.9});

	string_assert("result", serialized, R"([13.75,10,0,-0.9])");
}

void test_parse_1() {

	auto parsed = parse(" 42 ");
	auto serialized = stringify(parsed);

	string_assert("result", serialized, R"(42)");
}

void test_parse_2() {

	auto parsed = parse(" true");
	auto serialized = stringify(parsed);

	string_assert("result", serialized, R"(true)");
}

void test_parse_3() {

	auto parsed = parse("null");
	auto serialized = stringify(parsed);

	string_assert("result", serialized, R"(null)");
}

void test_parse_4() {

	auto parsed = parse("3.14");
	auto serialized = stringify(parsed);

	string_assert("result", serialized, R"(3.14)");
}

void test_parse_5() {

	auto parsed = parse(" [0, -4, 12.5 ]");
	auto serialized = stringify(parsed);

	string_assert("result", serialized, R"([0,-4,12.5])");
}

void test_parse_6() {

	auto parsed = parse(R"(
		{
			"brand": "mazda" ,
			"model" :"miata",
			"year": 2024,
			"hp": 185.3,
			"hard_top":true,
			"color": "skyblue"
		}
	)");

	auto serialized = stringify(parsed);

	string_assert("result", serialized, R"({"brand":"mazda","color":"skyblue","hard_top":true,"hp":185.3,"model":"miata","year":2024})");
}

void test_parse_7() {

	auto parsed = parse(R"(
		{
			"value": "test\\ttext here",
			"number": 4575889,
			"float": 16.4,
			"booolean": true,
			"object": {
				"key":"value",
				"flag": true
			},
			"array": [8,9,10],
			"strings": ["one","two","sever"],
			"objarray": [
				{
					"value": "succ\\"ess"
				},
				{
					"value": 14
				}
			]
		}
	)");

	auto serialized = stringify(parsed);

	string_assert("result", serialized, R"({"array":[8,9,10],"booolean":true,"float":16.4,"number":4575889,"objarray":[{"value":"succ\"ess"},{"value":14}],"object":{"flag":true,"key":"value"},"strings":["one","two","sever"],"value":"test\\ttext here"})");
}

int main() {

	auto queue = TestQueue({
		{ "stringify_1", test_stringify_1 },
		{ "stringify_2", test_stringify_2 },
		{ "stringify_3", test_stringify_3 },
		{ "stringify_4", test_stringify_4 },
		{ "parse_1", test_parse_1 },
		{ "parse_2", test_parse_2 },
		{ "parse_3", test_parse_3 },
		{ "parse_4", test_parse_4 },
		{ "parse_5", test_parse_5 },
		{ "parse_6", test_parse_6 },
		{ "parse_7", test_parse_7 },
	});

	return exec_queue(queue);
}
