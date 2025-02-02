#include "./base64.hpp"

using namespace Lambda::Encoding;

bool Base64::validate(const std::string& data) {

	for (const auto rune : data) {
		if (!isalnum(rune) && rune != '+' && rune != '/' && rune != '=') {
			return false;
		}
	}

	return true;
}

//	full decode table does brrrrrr. high-speed tricks here
static const uint8_t table_decode[] = {
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

#define dec_b1(dest, block, byte_in, byte_next) \
	((dest[(block)]) = (table_decode[((uint8_t)(byte_in))] ^ (table_decode[((uint8_t)(byte_next))] >> 6)))

#define dec_b2(dest, block, byte_in, byte_next) \
	((dest[((block) + 1)]) = (table_decode[((uint8_t)(byte_in))] << 2) ^ (table_decode[((uint8_t)(byte_next))] >> 4))

#define dec_b3(dest, block, byte_in, byte_next) \
	((dest[((block) + 2)]) = (table_decode[((uint8_t)(byte_in))] << 4) ^ (table_decode[((uint8_t)(byte_next))] >> 2))

std::vector<uint8_t> Base64::decode(const std::string& data) {

	size_t have_blocks = ((data.size() / 4) * 4);
	size_t result_blocks = ((data.size() / 4) * 3);

	std::vector<uint8_t> result;
	result.resize(result_blocks);

	for (size_t r_idx = 0, w_idx = 0; r_idx < have_blocks; r_idx += 4, w_idx += 3) {
		dec_b1(result, w_idx, data[r_idx], data[r_idx + 1]);
		dec_b2(result, w_idx, data[r_idx + 1], data[r_idx + 2]);
		dec_b3(result, w_idx, data[r_idx + 2], data[r_idx + 3]);
	}

	size_t incomplete_bytes = (data.size() - have_blocks);

	if (incomplete_bytes > 0) {

		result.resize(result.size() + 1);

		if (incomplete_bytes == 3) {
			dec_b1(result, result_blocks, data[have_blocks], data[have_blocks + 1]);
			dec_b2(result, result_blocks, data[have_blocks + 1], data[have_blocks + 2]);
		} else if (incomplete_bytes == 2) {
			dec_b1(result, result_blocks, data[have_blocks], data[have_blocks + 1]);
			dec_b2(result, result_blocks, data[have_blocks + 1], 0);
		} else {
			dec_b1(result, result_blocks, data[have_blocks], 0);
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
static const char table_encode[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

#define enc_b1(dest, block, byte_in) \
	((dest[(block)]) = (table_encode[((((uint8_t)(byte_in)) >> 2) & 0x3F)]));

#define enc_b2(dest, block, byte_in, byte_next) \
	((dest[((block) + 1)]) = (table_encode[(((((uint8_t)(byte_in)) << 4) ^ (((uint8_t)(byte_next)) >> 4)) & 0x3F)]))

#define enc_b3(dest, block, byte_in, byte_next) \
	((dest[((block) + 2)]) = (table_encode[(((((uint8_t)(byte_in)) << 2) ^ ((uint8_t)(byte_next)) >> 6) & 0x3F)]))

#define enc_b4(dest, block, byte_next) \
	((dest[((block) + 3)]) = (table_encode[(((uint8_t)(byte_next)) & 0x3F)]))

std::string Base64::encode(const std::vector<uint8_t>& data) {

	size_t have_blocks = (data.size() / 3) * 3;
	size_t result_blocks = ((have_blocks * 4) / 3);

	std::string result;
	result.resize(result_blocks);
	
	for (size_t r_idx = 0, w_idx = 0; r_idx < have_blocks; r_idx += 3, w_idx += 4) {
		enc_b1(result, w_idx, data[r_idx]);
		enc_b2(result, w_idx, data[r_idx], data[r_idx + 1]);
		enc_b3(result, w_idx, data[r_idx + 1], data[r_idx + 2]);
		enc_b4(result, w_idx, data[r_idx + 2]);
	}
		
	size_t incomplete_bytes = (data.size() - have_blocks);
	if (incomplete_bytes == 0) {
		return result;
	}
	
	result.resize(result.size() + 4, '=');

	if (incomplete_bytes > 0) {
		enc_b1(result, result_blocks, data[have_blocks]);

		if (incomplete_bytes == 1) {
			enc_b2(result, result_blocks, data[have_blocks], 0);
		} else {
			enc_b2(result, result_blocks, data[have_blocks], data[have_blocks + 1]);
			enc_b3(result, result_blocks, data[have_blocks + 1], 0);
		}
	}

	return result;
}
