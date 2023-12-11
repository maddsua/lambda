#include "../encoding.hpp"

#include <map>
#include <stdexcept>

using namespace Lambda;

//	full decode table does brrrrrr. high-speed tricks here
static const uint8_t base64DecodeTable[] = {
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,
	0xF8,
	0,0,0,
	0xFC,0xD0,0xD4,0xD8,0xDC,0xE0,0xE4,0xE8,
	0xEC,0xF0,0xF4,
	0,0,0,0,0,0,0,
	0x00,0x04,0x08,0x0C,0x10,0x14,0x18,0x1C,
	0x20,0x24,0x28,0x2C,0x30,0x34,0x38,0x3C,
	0x40,0x44,0x48,0x4C,0x50,0x54,0x58,0x5C,
	0x60,0x64,
	0,0,0,0,0,0,
	0x68,0x6C,0x70,0x74,0x78,0x7C,0x80,0x84,
	0x88,0x8C,0x90,0x94,0x98,0x9C,0xA0,0xA4,
	0xA8,0xAC,0xB0,0xB4,0xB8,0xBC,0xC0,0xC4,
	0xC8,0xCC,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0
};

#define decB1(buffOut, posOut, byteIn1, byteIn2) \
	((buffOut[(posOut)]) = (base64DecodeTable[((uint8_t)(byteIn1))] ^ (base64DecodeTable[((uint8_t)(byteIn2))] >> 6)))

#define decB2(buffOut, posOut, byteIn1, byteIn2) \
	((buffOut[((posOut) + 1)]) = (base64DecodeTable[((uint8_t)(byteIn1))] << 2) ^ (base64DecodeTable[((uint8_t)(byteIn2))] >> 4))

#define decB3(buffOut, posOut, byteIn1, byteIn2) \
	((buffOut[((posOut) + 2)]) = (base64DecodeTable[((uint8_t)(byteIn1))] << 4) ^ (base64DecodeTable[((uint8_t)(byteIn2))] >> 2))

std::string Encoding::fromBase64(const std::string& data) {

	//	check base64 string validity real quick
	for (size_t i = 0; i < data.size(); i++) {
		auto symbol = data[i];
		if (!isalnum(symbol) && symbol != '+' && symbol != '/' && symbol != '=')
			throw std::runtime_error("Invalid symbol in base64 sequence at position " + std::to_string(i));
	}

	size_t encodedCompleteBlocks = ((data.size() / 4) * 4);
	size_t outputCompleteBlocks = ((data.size() / 4) * 3);

	std::string result;
	result.resize(outputCompleteBlocks);

	for (size_t encIdx = 0, dataIdx = 0; encIdx < encodedCompleteBlocks; encIdx += 4, dataIdx += 3) {
		decB1(result, dataIdx, data[encIdx], data[encIdx + 1]);
		decB2(result, dataIdx, data[encIdx + 1], data[encIdx + 2]);
		decB3(result, dataIdx, data[encIdx + 2], data[encIdx + 3]);
	}

	size_t incompleteBlockSize = (data.size() - encodedCompleteBlocks);

	if (incompleteBlockSize > 0) {

		result.resize(result.size() + 1);

		if (incompleteBlockSize == 3) {
			decB1(result, outputCompleteBlocks, data[encodedCompleteBlocks], data[encodedCompleteBlocks + 1]);
			decB2(result, outputCompleteBlocks, data[encodedCompleteBlocks + 1], data[encodedCompleteBlocks + 2]);
		} else if (incompleteBlockSize == 2) {
			decB1(result, outputCompleteBlocks, data[encodedCompleteBlocks], data[encodedCompleteBlocks + 1]);
			decB2(result, outputCompleteBlocks, data[encodedCompleteBlocks + 1], 0);
		} else {
			decB1(result, outputCompleteBlocks, data[encodedCompleteBlocks], 0);
		}

		return result;
	}

	uint8_t paddLength = 0;
	for (uint8_t i = 1; i <= 2; i++) {
		if (data[data.size() - i] == '=')
			paddLength++;
	}

	if (paddLength > 0) 
		result.resize(result.size() - paddLength);

	return result;
}


//	short encode table
static const char base64EncodeTable[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

#define encB1(buffOut, posOut, byteIn1) \
	((buffOut[(posOut)]) = (base64EncodeTable[((((uint8_t)(byteIn1)) >> 2) & 0x3F)]));

#define encB2(buffOut, posOut, byteIn1, byteIn2) \
	((buffOut[((posOut) + 1)]) = (base64EncodeTable[(((((uint8_t)(byteIn1)) << 4) ^ (((uint8_t)(byteIn2)) >> 4)) & 0x3F)]))

#define encB3(buffOut, posOut, byteIn1, byteIn2) \
	((buffOut[((posOut) + 2)]) = (base64EncodeTable[(((((uint8_t)(byteIn1)) << 2) ^ ((uint8_t)(byteIn2)) >> 6) & 0x3F)]))

#define encB4(buffOut, posOut, byteIn2) \
	((buffOut[((posOut) + 3)]) = (base64EncodeTable[(((uint8_t)(byteIn2)) & 0x3F)]))

std::string Encoding::toBase64(const std::string& data) {

	size_t dataCompleteBlocks = (data.size() / 3) * 3;
	size_t outputCompleteBlocks = ((dataCompleteBlocks * 4) / 3);

	std::string result;
	result.resize(outputCompleteBlocks);
	
	for (size_t dataIdx = 0, encIdx = 0; dataIdx < dataCompleteBlocks; dataIdx += 3, encIdx += 4) {
		encB1(result, encIdx, data[dataIdx]);
		encB2(result, encIdx, data[dataIdx], data[dataIdx + 1]);
		encB3(result, encIdx, data[dataIdx + 1], data[dataIdx + 2]);
		encB4(result, encIdx, data[dataIdx + 2]);
	}
		
	size_t incompleteBlockSize = (data.size() - dataCompleteBlocks);
	if (incompleteBlockSize == 0) return result;
	
	result.resize(result.size() + 4, '=');

	if (incompleteBlockSize > 0) {
		encB1(result, outputCompleteBlocks, data[dataCompleteBlocks]);

		if (incompleteBlockSize == 1) {
			encB2(result, outputCompleteBlocks, data[dataCompleteBlocks], 0);
		} else {
			encB2(result, outputCompleteBlocks, data[dataCompleteBlocks], data[dataCompleteBlocks + 1]);
			encB3(result, outputCompleteBlocks, data[dataCompleteBlocks + 1], 0);
		}
	}

	return result;
}
