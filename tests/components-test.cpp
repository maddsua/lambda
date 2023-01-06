#include <iostream>
#include <string>

using namespace std;

#include "../include/maddsua/compress.hpp"
#include "../include/maddsua/fs.hpp"
#include "../include/maddsua/base64.hpp"

int main(int argc, char** argv) {

	//	filesystem

	cout << "\r\nLoading data sample...\r\n";

	string test_data;

	if (maddsua::readBinary("../demo/index.html", &test_data)) {
		cout << to_string(test_data.size()) << " bytes OK";
	} else {
		cout << "FAILED";
		return 1;
	}


	//	base64 decoding / encoding
	{
		cout << "\r\n\r\nTesting base64 encoder/decoder...\r\nEncoding ";

		auto base64EncodedData = maddsua::b64Encode(test_data);
		cout << (maddsua::b64Validate(&base64EncodedData) ? "OK" : "FAILED");

		cout << ", Decompression ";
		if (maddsua::b64Decode(base64EncodedData) == test_data) {
			cout <<  "OK";
		} else {
			cout << "FAILED";
			return 1;
		}
	}


	//	zlib test
	{
		cout << "\r\n\r\nTesting zlib gzip compression...\r\nCompressing ";

		std::string encoded;
		if (maddsua::gzCompress(&test_data, &encoded, true)) {
			cout <<  "OK";
		} else {
			cout << "FAILED";
			return 1;
		}

		cout << ", Decoding: ";
		std::string decoded;
		if (maddsua::gzDecompress(&encoded, &decoded)) {
			cout <<  "OK";
		} else {
			cout << "FAILED";
			return 1;
		}

		cout << "\r\nData matched? ";
		if (decoded == test_data) {
			cout <<  "YES";
		} else {
			cout << "NO";
			return 1;
		}
	}


	//	brotli test
	{
		cout << "\r\n\r\nTesting brotli compression...\r\nCompressing ";

		std::string encoded;
		if (maddsua::brCompress(&test_data, &encoded)) {
			cout <<  "OK";
		} else {
			cout << "FAILED";
			return 1;
		}

		cout << ", Decoding: ";
		std::string decoded;
		if (maddsua::brDecompress(&encoded, &decoded)) {
			cout <<  "OK";
		} else {
			cout << "FAILED";
			return 1;
		}

		cout << "\r\nData matched? ";
		if (decoded == test_data) {
			cout <<  "YES";
		} else {
			cout << "NO";
			return 1;
		}
	}

	cout << "\r\n\r\nComponent test OK\r\n";

	return 0;
}