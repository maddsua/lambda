#include "./json.hpp"

#include <stdexcept>

using namespace Lambda;
using namespace Lambda::JSON;

void Property::clone(const Property& other) {
	this->internalType = other.internalType;
	if (other.internalDataBool) this->internalDataBool = new bool(*other.internalDataBool);
	if (other.internalDataFloat) this->internalDataFloat = new long double(*other.internalDataFloat);
	if (other.internalDataInt) this->internalDataInt = new int32_t(*other.internalDataInt);
	if (other.internalDataStr) this->internalDataStr = new std::string(*other.internalDataStr);
	if (other.internalDataArray) this->internalDataArray = new std::vector<Property>(*other.internalDataArray);
	if (other.internalDataMap) this->internalDataMap = new std::unordered_map<std::string, Property>(*other.internalDataMap);
}

Property::Property() {
	this->internalType = JSTypes::Undefined;
}

Property::Property(std::nullptr_t value) {
	this->internalType = JSTypes::Null;
}

Property::Property(long double value) {
	this->internalType = JSTypes::Float;
	this->internalDataFloat = new long double(value);
}

Property::Property(double value) {
	this->internalType = JSTypes::Float;
	this->internalDataFloat = new long double(value);
}

Property::Property(float value) {
	this->internalType = JSTypes::Float;
	this->internalDataFloat = new long double(value);
}

Property::Property(int32_t value) {
	this->internalType = JSTypes::Integer;
	this->internalDataInt = new int32_t(value);
}

Property::Property(uint32_t value) {
	this->internalType = JSTypes::Integer;
	this->internalDataInt = new int32_t(value);
}

Property::Property(int16_t value) {
	this->internalType = JSTypes::Integer;
	this->internalDataInt = new int32_t(value);
}

Property::Property(uint16_t value) {
	this->internalType = JSTypes::Integer;
	this->internalDataInt = new int32_t(value);
}

Property::Property(bool value) {
	this->internalType = JSTypes::Boolean;
	this->internalDataBool = new bool(value);
}

Property::Property(const std::string& value) {
	this->internalType = JSTypes::String;
	this->internalDataStr = new std::string(value);
}

Property::Property(const char* value) {
	this->internalType = JSTypes::String;
	this->internalDataStr = new std::string(value);
}

Property::Property(const std::vector<Property>& value) {
	this->internalType = JSTypes::Array;
	this->internalDataArray = new std::vector<Property>(value);
}

Property::Property(const std::unordered_map<std::string, Property>& value) {
	this->internalType = JSTypes::Map;
	this->internalDataMap = new std::unordered_map<std::string, Property>(value);
}

Property::Property(const Property& other) {
	this->clone(other);
}

Property::Property(Property&& other) {
	this->clone(other);
}

Property Property::operator=(const Property& other) {
	this->clone(other);
	return *this;
}

JSTypes Property::type() const {
	return this->internalType;
}

bool Property::isUndefined() const {
	return this->internalType == JSTypes::Undefined;
}

bool Property::isNull() const {
	return this->internalType == JSTypes::Null;
}

bool Property::isNil() const {
	return this->internalType == JSTypes::Null || this->internalType == JSTypes::Undefined;
}

long double Property::asFloat() const {
	if (this->internalType != JSTypes::Float)
		throw std::runtime_error("TypeError: value is not a float");
	else if (!this->internalDataFloat)
		throw std::runtime_error("DataError: float value undefined");
	return *this->internalDataFloat;
}

int32_t Property::asInt() const {
	if (this->internalType != JSTypes::Integer)
		throw std::runtime_error("TypeError: value is not an int");
	else if (!this->internalDataInt)
		throw std::runtime_error("DataError: int value undefined");
	return *this->internalDataInt;
}

bool Property::asBool() const {
	if (this->internalType != JSTypes::Boolean)
		throw std::runtime_error("TypeError: value is not a boolean");
	else if (!this->internalDataBool)
		throw std::runtime_error("DataError: boolean value undefined");
	return *this->internalDataBool;
}

const std::string& Property::asString() const {
	if (this->internalType != JSTypes::String)
		throw std::runtime_error("TypeError: value is not a string");
	else if (!this->internalDataStr)
		throw std::runtime_error("DataError: string value undefined");
	return *this->internalDataStr;
}

const std::vector<Property>& Property::asArray() const {
	if (this->internalType != JSTypes::Array)
		throw std::runtime_error("TypeError: value is not an array");
	else if (!this->internalDataArray)
		throw std::runtime_error("DataError: array value undefined");
	return *this->internalDataArray;
}

const std::unordered_map<std::string, Property>& Property::asMap() const {
	if (this->internalType != JSTypes::Map)
		throw std::runtime_error("TypeError: value is not a map");
	else if (!this->internalDataMap)
		throw std::runtime_error("DataError: map value undefined");
	return *this->internalDataMap;
}

Property::~Property() {
	if (this->internalDataBool) delete this->internalDataBool;
	if (this->internalDataInt) delete this->internalDataInt;
	if (this->internalDataFloat) delete this->internalDataFloat;
	if (this->internalDataStr) delete this->internalDataStr;
	if (this->internalDataArray) delete this->internalDataArray;
	if (this->internalDataMap) delete this->internalDataMap;
}
