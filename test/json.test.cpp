#include <iostream>
#include <unordered_map>

#include "../extra/json.hpp"

using namespace JSON;

int main(int argc, char const *argv[]) {

	/*std::cout << "Stringify 1: " << stringify(Property()) << "\n";
	std::cout << "Stringify 2: " << stringify(Property("Hi there!")) << "\n";
	std::cout << "Stringify 3: " << stringify(Property(std::string("Hi there!"))) << "\n";
	std::cout << "Stringify 4: " << stringify(Property(12.86)) << "\n";
	std::cout << "Stringify 5: " << stringify(Property(25)) << "\n";
	std::cout << "Stringify 6: " << stringify(Property(true)) << "\n";
	std::cout << "Stringify 7: " << stringify(Property(nullptr)) << "\n";*/

	JSON::Array testArray = {
		Property(8),
		Property(3),
		Property(150),
		Property(9),
		Property(JSON::Map({
			{"record", Property(77)},
			{"record_2", Property(150000)},
		}))
	};

	std::cout << "Stringify 8: " << stringify(Property(testArray)) << "\n";

	JSON::Map testMap = {
		{"record", Property(8)},
		{"record_2", Property(150)},
		{"record_3", Property("Look here")},
		{"record_4", Property(true)},
		{"record_5", Property(testArray)},
	};

	std::cout << "Stringify 9: " << stringify(Property(testMap)) << "\n";

	std::cout << "\n---\n";

	std::cout << "Get 1: " << Property().isUndefined() << "\n";
	std::cout << "Get 2: " << Property("Hi there!").asString() << "\n";
	std::cout << "Get 3: " << Property(std::string("Hi there!")).asString() << "\n";
	std::cout << "Get 4: " << Property(12.86).asFloat() << "\n";
	std::cout << "Get 5: " << Property(25).asInt() << "\n";
	std::cout << "Get 6: " << Property(true).asBool() << "\n";
	std::cout << "Get 7: " << Property(nullptr).isNull() << "\n";

	std::cout << "\n---\n";

	std::cout << "Restringify 1: " << stringify(Property(parse("true"))) << "\n";
	std::cout << "Restringify 2: " << stringify(Property(parse("text"))) << "\n";
	std::cout << "Restringify 3: " << stringify(Property(parse("\"text\""))) << "\n";
	std::cout << "Restringify 4: " << stringify(Property(parse("null"))) << "\n";
	std::cout << "Restringify 5: " << stringify(Property(parse("8888"))) << "\n";
	std::cout << "Restringify 6: " << stringify(Property(parse("[8,3,150,9]"))) << "\n";
	std::cout << "Restringify 6: " << stringify(Property(parse("{\"record_5\":[8,3,150,9],\"record_4\":true,\"record_3\":\"Look here\n\tboo!\",\"record_2\":150,\"record\":8}"))) << "\n";

	std::cout << "\n---\n";

	std::string datajson = R"(
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
					"value": "success"
				},
				{
					"value": 14
				}
			]
		}
	)";

	auto comprexObject = JSON::parse(datajson);

	std::cout << "Read 1: " << comprexObject.asMap().find("value")->second.asString() << "\n";
	std::cout << "Read 2: " << comprexObject.asMap().find("number")->second.asInt() << "\n";
	std::cout << "Read 3: " << comprexObject.asMap().find("float")->second.asFloat() << "\n";

	return 0;
}
