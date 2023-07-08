#include "./encoding.hpp"
#include "../lambda_private.hpp"

using namespace Lambda;
using namespace Lambda::Encoding;

void tabLeftpad(std::string& content) {

	std::vector<size_t> linebreaks;
	bool toggle = false;

	for (size_t i = 0; i < content.size(); i++) {

		//	get rid of CR
		if (content[i] == '\r') {
			content.erase(i, 1);
			i--;
			continue;
		}

		// leftpad of some sort
		if (!toggle && content[i] == '\n') {
			linebreaks.push_back(i);
			toggle = true;
		} else if (toggle && content[i] != '\n' && content[i] != '\r') {
			toggle = false;
		}
	}
	
	for (size_t i = 0; i < linebreaks.size(); i++) {
		content.insert(linebreaks[i] + i + 1, "\t");
	}
}

void JSON_Object::addString(const std::string& key, const std::string& value) {
	this->data[key] = "\"" + value + "\"";
}

void JSON_Object::addInt(const std::string& key, const int64_t value) {
	this->data[key] = std::to_string(value);
}

void JSON_Object::addInt(const std::string& key, const int32_t value) {
	this->data[key] = std::to_string(value);
}

void JSON_Object::addFlt(const std::string& key, const float value) {
	this->data[key] = std::to_string(value);
}

void JSON_Object::addFlt(const std::string& key, const double value) {
	this->data[key] = std::to_string(value);
}

void JSON_Object::addBool(const std::string& key, const bool value) {
	this->data[key] = value ? "true" : "false";
}

void JSON_Object::addStringArray(const std::string& key, const std::vector<std::string>& value) {
	std::string temp;
	for (const auto& item : value) {
		if (temp.size() > 0) temp += ", ";
		temp += "\"" + item + "\"";
	}
	this->data[key] = "[" + temp + "]";
}

void JSON_Object::addIntArray(const std::string& key, const std::vector<int64_t> value) {
	std::string temp;
	for (const auto& item : value) {
		if (temp.size() > 0) temp += ", ";
		temp += std::to_string(item);
	}
	this->data[key] = "[" + temp + "]";
}

void JSON_Object::addFltArray(const std::string& key, const std::vector<double> value) {
	std::string temp;
	for (const auto& item : value) {
		if (temp.size() > 0) temp += ", ";
		temp += std::to_string(item);
	}
	this->data[key] = "[" + temp + "]";
}


void JSON_Object::addBoolArray(const std::string& key, const std::vector<bool> value) {
	std::string temp;
	for (const auto& item : value) {
		if (temp.size() > 0) temp += ", ";
		temp += item ? "true" : "false";
	}
	this->data[key] = "[" + temp + "]";
}

void JSON_Object::addObject(const std::string& key, std::string objectString) {
	tabLeftpad(objectString);
	this->data[key] = objectString;
}

void JSON_Object::addObject(const std::string& key, JSON_Object& object) {
	addObject(key, object.stringify());
}

void JSON_Object::addObject(const std::string& key, JSON_Array& object) {
	addObject(key, object.stringify());
}

std::string JSON_Object::stringify() {
	std::string temp;
	for (const auto& item : this->data) {
		if (temp.size() > 0) temp += ",\n";
		temp += "\t\"" + item.first + "\": " + item.second;
	}
	return "{\n" + temp + "\n}";
}

void JSON_Object::addNull(const std::string& key) {
	this->data[key] = "null";
}

void JSON_Object::removeKey(const std::string& key) {
	if (!this->data.contains(key)) return;
	this->data.erase(key);
}

JSON_Object JSON_Object::operator+=(const JSON_Object& b) {
	
	for (const auto& item : b.data) {
		if (this->data.contains(item.first)) continue;
		this->data[item.first] = item.second;
	}
	
	return *this;
}


void JSON_Array::pushString(const std::string& value) {
	this->data.push_back("\"" + value + "\"");
}

void JSON_Array::pushInt(int64_t value) {
	this->data.push_back(std::to_string(value));
}

void JSON_Array::pushInt(int32_t value) {
	this->data.push_back(std::to_string(value));
}

void JSON_Array::pushFlt(float value) {
	this->data.push_back(std::to_string(value));
}

void JSON_Array::pushFlt(double value) {
	this->data.push_back(std::to_string(value));
}

void JSON_Array::pushBool(bool value) {
	this->data.push_back(value ? "true" : "false");
}

void JSON_Array::push(JSON_Object& object) {
	auto temp = object.stringify();
	tabLeftpad(temp);
	this->data.push_back(temp);
}

void JSON_Array::push(JSON_Array& object) {
	auto temp = object.stringify();
	tabLeftpad(temp);
	this->data.push_back(temp);
}


std::string JSON_Array::stringify() {
	std::string temp;
	for (const auto& item : this->data) {
		if (temp.size() > 0) temp += ",\n\t";
		temp += item;
	}
	return "[\n\t" + temp + "\n]";
}


JSON_Array JSON_Array::operator+=(const JSON_Array& b) {
	this->data.reserve(this->data.size() + b.data.size());
	for (const auto& item : b.data) {
		this->data.push_back(item);
	}
	return *this;
}

JSONParser::JSONParser(const std::string& json) {

	this->data.reserve(json.size());
	for (size_t i = 0; i < json.size(); i++) {
		if (json[i] == '\r' || json[i] == '\n' || json[i] == '\t')
			continue;
		this->data.push_back(json[i]);
	}

	size_t pos_obj = std::string::npos;
	size_t pos_arr = std::string::npos;

	for (size_t i = 1; i < this->data.size(); i++) {

		if (this->data.at(i - 1) == '\\') continue;

		const auto& c = this->data.at(i);

		if (c == '{' && pos_obj == std::string::npos) {
			pos_obj = i;
		} else if (c == '}' && pos_obj != std::string::npos) {
			this->mask.push_back({pos_obj, i + 1});
			pos_obj = std::string::npos;
		} else if (c == '[' && pos_arr == std::string::npos) {
			pos_arr = i;
		} else if (c == ']' && pos_arr != std::string::npos) {
			this->mask.push_back({pos_arr, i + 1});
			pos_arr = std::string::npos;
		}
	}
}

size_t JSONParser::findPropNext(const std::string& key) {

	std::string pattern = "\"" + key + "\"";

	size_t searchIdx = std::string::npos;
	size_t searchStart = 0;

	while (searchIdx == std::string::npos) {

		auto tempIdx = this->data.find(pattern, searchStart);
		if (tempIdx == searchIdx) throw Lambda::Error("Prop \"" + key + "\" not found");
		
		for (const auto& holdout : this->mask) {

			if (holdout.from > tempIdx) break;

			if (tempIdx < holdout.to)  {
				tempIdx = searchIdx;
				searchStart = holdout.to;
				break;
			}
		}

		searchIdx = tempIdx;
	}

	return searchIdx + pattern.size();
}


int64_t JSONParser::getIntProp(const std::string& key) {

	auto propnext = findPropNext(key);

	bool flagPropSeparator = false;
	bool flagNumberBegan = false;

	std::string numericValue;
	numericValue.reserve(8);

	for (size_t i = propnext; i < this->data.size(); i++) {

		const auto& c = this->data[i];

		if (flagPropSeparator) {
			if (c == ' ') continue;
			else if (c >= '0' || c <= '9') flagNumberBegan = true;
			else throw Lambda::Error("JSON parsing failed at symbol " + std::to_string(i) + " on key: \"" + key + "\"");
		} else {
			if (c == ' ') continue;
			else if (c == ':') flagPropSeparator = true;
			else throw Lambda::Error("JSON parsing failed at symbol " + std::to_string(i) + " on key: \"" + key + "\"");
		}

		if (flagNumberBegan) {
			if (c < '0' || c > '9') break;
			numericValue.push_back(c);
		}
	}

	if (!numericValue.size()) Lambda::Error("JSON parsing failed: empty numeric value on key: \"" + key + "\"");

	try {
		return std::stoll(numericValue);
	} catch(...) {
		throw Lambda::Error("JSON parsing failed: could not parse int on key: \"" + key + "\"");
	}
}

long double JSONParser::getFltProp(const std::string& key) {

	auto propnext = findPropNext(key);

	bool flagPropSeparator = false;
	bool flagNumberBegan = false;

	std::string numericValue;
	numericValue.reserve(8);

	for (size_t i = propnext; i < this->data.size(); i++) {

		const auto& c = this->data.at(i);

		if (flagPropSeparator) {
			if (c == ' ') continue;
			else if (c >= '0' || c <= '9') flagNumberBegan = true;
			else throw Lambda::Error("JSON parsing failed at symbol " + std::to_string(i) + " on key: \"" + key + "\"");
		} else {
			if (c == ' ') continue;
			else if (c == ':') flagPropSeparator = true;
			else throw Lambda::Error("JSON parsing failed at symbol " + std::to_string(i) + " on key: \"" + key + "\"");
		}

		if (flagNumberBegan) {
			if ((c < '0' || c > '9') && c != '.') break;
			numericValue.push_back(c);
		}
	}

	if (!numericValue.size()) Lambda::Error("JSON parsing failed: empty numeric value on key: \"" + key + "\"");

	return std::stold(numericValue);
}

std::string JSONParser::getStringProp(const std::string& key) {

	auto propnext = findPropNext(key);

	bool flagPropSeparator = false;
	size_t pos_start = std::string::npos;

	for (size_t i = propnext; i < this->data.size(); i++) {

		const auto& c = this->data.at(i);

		if (pos_start != std::string::npos) {

			if (c == '\"' &&  this->data.at(i - 1) != '\\')
				return this->data.substr(pos_start, i - pos_start);

		} else if (flagPropSeparator) {
			if (c == ' ') continue;
			else if (c == '\"') pos_start = i + 1;
			else throw Lambda::Error(std::string("JSON parsing failed: unexpected token ") + c + " on key: \"" + key + "\"");
		} else {
			if (c == ' ') continue;
			else if (c == ':') flagPropSeparator = true;
			else throw Lambda::Error(std::string("JSON parsing failed: unexpected token ") + c + " on key: \"" + key + "\"");
		}
	}

	throw Lambda::Error("JSON parsing failed: unexpected logic on key: \"" + key + "\"");
}


JSONParser JSONParser::getObjectProp(const std::string& key) {

	auto propnext = findPropNext(key);

	bool flagPropSeparator = false;
	size_t pos_start = std::string::npos;
	size_t ptnesses = 1;

	for (size_t i = propnext; i < this->data.size(); i++) {

		const auto& c = this->data.at(i);

		if (pos_start != std::string::npos) {
			
			if (c == '{') ptnesses++;
			else if (c == '}' && this->data.at(i - 1) != '\\') ptnesses--;

			if (ptnesses == 0) return this->data.substr(pos_start, i - pos_start + 1);

		} else if (flagPropSeparator) {
			if (c == ' ') continue;
			else if (c == '{') pos_start = i;
			else throw Lambda::Error(std::string("JSON parsing failed: unexpected token ") + c + " on key \"" + key + "\"");
		} else {
			if (c == ' ') continue;
			else if (c == ':') flagPropSeparator = true;
			else throw Lambda::Error(std::string("JSON parsing failed: unexpected token ") + c + " on key \"" + key + "\"");
		}
	}

	throw Lambda::Error("JSON parsing failed: unexpected logic on key: \"" + key + "\"");
}

bool JSONParser::getBoolProp(const std::string& key) {

	auto propnext = findPropNext(key);

	bool flagPropSeparator = false;

	for (size_t i = propnext; i < this->data.size(); i++) {

		const auto& c = this->data[i];

		if (flagPropSeparator) {
			if (c == ' ') continue;
			else if (c == 't' || c == 'f') return c == 't';
			else throw Lambda::Error("JSON parsing failed at symbol " + std::to_string(i) + " on key: \"" + key + "\"");
		} else {
			if (c == ' ') continue;
			else if (c == ':') flagPropSeparator = true;
			else throw Lambda::Error("JSON parsing failed at symbol " + std::to_string(i) + " on key: \"" + key + "\"");
		}
	}

	return false;
}


std::string JSONParser::selectArray(const std::string& key) {
	
	auto propnext = findPropNext(key);

	bool flagPropSeparator = false;
	size_t pos_start = std::string::npos;
	size_t ptnesses = 1;

	for (size_t i = propnext; i < this->data.size(); i++) {

		const auto& c = this->data.at(i);

		if (pos_start != std::string::npos) {
			
			if (c == '[') ptnesses++;
			else if (c == ']' && this->data.at(i - 1) != '\\') ptnesses--;

			if (ptnesses == 0) return this->data.substr(pos_start, i - pos_start + 1);

		} else if (flagPropSeparator) {
			if (c == ' ') continue;
			else if (c == '[') pos_start = i;
			else throw Lambda::Error(std::string("JSON parsing failed: unexpected token ") + c + " on key \"" + key + "\"");
		} else {
			if (c == ' ') continue;
			else if (c == ':') flagPropSeparator = true;
			else throw Lambda::Error(std::string("JSON parsing failed: unexpected token ") + c + " on key \"" + key + "\"");
		}
	}

	throw Lambda::Error("JSON parsing failed: unexpected logic on key: \"" + key + "\"");
}

std::vector<uint64_t> JSONParser::getIntArrayProp(const std::string& key) {

	auto arrayProp = selectArray(key);

	std::vector<uint64_t> result;
	std::string temp;
	temp.reserve(8);

	for (const char& c : arrayProp) {

		if (c >= '0' && c <= '9') {
			temp.push_back(c);
			continue;
		}

		if (!temp.size()) continue;

		try {
			result.push_back(std::stoll(temp));
			temp.clear();
		} catch(...) {
			throw Lambda::Error("JSON parsing failed: could not parse int  on key: \"" + key + "\"");
		}
		
	}

	return result;
}

std::vector<long double> JSONParser::getFltArrayProp(const std::string& key) {

	auto arrayProp = selectArray(key);

	std::vector<long double> result;
	std::string temp;
	temp.reserve(8);

	for (const char& c : arrayProp) {

		if (c >= '0' && c <= '9' || c == '.') {
			temp.push_back(c);
			continue;
		}

		if (!temp.size()) continue;

		try {
			result.push_back(std::stoll(temp));
			temp.clear();
		} catch(...) {
			throw Lambda::Error("JSON parsing failed: could not parse int  on key: \"" + key + "\"");
		}
		
	}

	return result;
}

std::vector<std::string> JSONParser::getStringArrayProp(const std::string& key) {

	auto arrayProp = selectArray(key);

	std::vector<std::string> result;

	size_t pos_start = std::string::npos;

	for (size_t i = 0; i < arrayProp.size(); i++) {

		const auto& c = arrayProp.at(i);
		
		if (c != '\"' || arrayProp.at(i - 1) == '\\') continue;

		if (pos_start == std::string::npos) {
			pos_start = i + 1;
			continue;
		}

		result.push_back(arrayProp.substr(pos_start, i - pos_start));
		pos_start = std::string::npos;
	}
	

	return result;
}

std::vector<JSONParser> JSONParser::getObjectArrayProp(const std::string& key) {

	auto arrayProp = selectArray(key);

	std::vector<JSONParser> result;

	size_t pos_start = std::string::npos;

	for (size_t i = 1; i < arrayProp.size(); i++) {

		const auto& c = arrayProp.at(i);

		if (c != '{' && c != '}' || arrayProp.at(i - 1) == '\\') continue;

		if (pos_start == std::string::npos && c == '{') {
			pos_start = i + 1;
			continue;
		}

		if (pos_start == std::string::npos || c != '}') continue;

		result.push_back(arrayProp.substr(pos_start, i - pos_start));
		pos_start = std::string::npos;
	}

	return result;
}
