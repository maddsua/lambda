#include "./kv.hpp"
#include "../encoding/encoding.hpp"
#include <fstream>
#include <cstring>
#include <iterator>

#include <iostream>

using namespace Lambda;
using namespace Lambda::Encoding;
using namespace Lambda::Storage;

const std::string magicString = "lambda kv json export";

Lambda::Error KV::exportJSON(const char* filepath) {

	auto localfile = std::ofstream(filepath);
	if (!localfile.is_open()) return Lambda::Error(std::string("Could not open file \"") + filepath + "\" for write");

	auto dataIsAsci = [](const std::string& data) {
		for (const auto& c : data) {
			if (c < 0x20 || c > 0x7d) return false;
		}
		return true;
	};
	
	JSON_Array exportData;

	std::lock_guard<std::mutex>lock(threadLock);

	for (const auto& entry : this->data) {

		auto key = entry.first;
		auto value = entry.second.value;
		bool base64 = false;
		bool key_base64 = false;

		if (!dataIsAsci(key)) {
			key = Encoding::b64Encode(key);
			key_base64 = true;
		}

		if (!dataIsAsci(value)) {
			value = Encoding::b64Encode(value);
			base64 = true;
		}

		JSON_Object record;
		record.addString("key", key);
		record.addString("value", value);
		record.addBool("base64", base64);
		record.addBool("key_base64", key_base64);
		record.addInt("created", entry.second.created);
		record.addInt("modified", entry.second.modified);
		exportData.push(record);
	}

	JSON_Object jsonExport;
	jsonExport.addString("type", magicString);
	jsonExport.addObject("entries", exportData);
	jsonExport.addInt("export_time", time(nullptr));
	jsonExport.addString("version", LAMBDA_VERSION);

	auto jsonExportRaw = jsonExport.stringify();
	localfile.write(jsonExportRaw.data(), jsonExportRaw.size());

	return {};
}

Lambda::Error KV::importJSON(const char* filepath) {

	std::string content;
	auto localfile = std::ifstream(filepath);
	if (!localfile.is_open()) return Lambda::Error(std::string("Could not open file \"") + filepath + "\" for read");

	content.assign(std::istreambuf_iterator<char>(localfile), std::istreambuf_iterator<char>());

	try {

		auto json = JSONParser(content);
		if (json.getStringProp("type") != magicString) return Lambda::Error(std::string("File \"") + filepath + "\" doen't seem to be a valid kv representation");

		auto entries = json.getObjectArrayProp("entries");

		for (auto& entry : entries) {

			KVMapEntry temp;
			temp.created = entry.getIntProp("created");
			temp.modified = entry.getIntProp("modified");
			temp.value = entry.getStringProp("value");

			auto key = entry.getStringProp("key");


			//	put this in try catch in case someone forgets to include these props
			try {
				if (entry.getBoolProp("base64"))
					temp.value = Encoding::b64Decode(temp.value);
			} catch(...) { }

			try {
				if (entry.getBoolProp("key_base64"))
					key = Encoding::b64Decode(key);
			} catch(...) { }

			std::lock_guard<std::mutex>lock(threadLock);
			this->data[key] = temp;
		}

	} catch(const std::exception& e) {
		return Lambda::Error(std::string("JSON import failed: ") + e.what());
	}

	return {};
}
