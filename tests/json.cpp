#include <iostream>
#include "../encoding/encoding.hpp"

using namespace Lambda::Encoding;

int main(void) {

	JSON_Object json;
	JSON_Object json2;
	JSON_Object json3;

	json3.addFlt("price", 16.99f);

	JSON_Array array;

	array.pushInt(44);
	array.pushFlt(44.2);
	array.pushBool(false);
	array.pushString("test");


	std::vector<int64_t> graph = {8,45,11,35};
	std::vector<std::string> tags = {"seo","web","landing","lead"};

	json2.addString("metadata", "false");
	json2.addIntArray("graph", graph);
	json2.addStringArray("tags", tags);

	array.push(json2);
	array.pushString("test");

	json.addBool("base64", false);
	json.addInt("size", 4096);
	json.addString("id", "test_record");
	json.addObject("user", json2);
	json.addObject("array", array);

	json += json3;

	std::cout << json.stringify() << std::endl;


	std::string datajson = R"(
		{
			"value": "test text here"
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
				},
			]
		}
	)";

	auto parsed = JSONParser(datajson);

	std::cout << "Bool vaue: " << parsed.getBoolProp("booolean") << std::endl;

}
