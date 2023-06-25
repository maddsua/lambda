#include "../../compression/compression.hpp"
#include <iostream>

int main() {

	std::cout << "\r\n--- Compression component test begin --- \r\n";

	std::cout << "\r\n--- zlib --- \r\n";

	auto zlib = Compress::ZlibStream();

	auto longText = std::string("this is a very long text. well, not really that long. repeat. this is a very long text. well, not really that long.");
	auto data = std::vector<uint8_t>(longText.begin(), longText.end());
	auto compressed = std::vector<uint8_t>();

	auto result = zlib.startCompression();
	printf("Startup of compression: %i\n", result);
	result = zlib.compressBuffer(&data, &compressed);
	printf("Compression: %i\n", result);
	printf("Sizes: %i/%i\n", data.size(), compressed.size());
	std::cout << std::endl;

	auto decompressed = std::vector<uint8_t>();
	result = zlib.startDecompression();
	printf("Startup of decompression: %i\n", result);
	result = zlib.decompressBuffer(&compressed, &decompressed);
	printf("Decompression: %i\n", result);
	printf("Data matched: %i\n", decompressed == data);

	return 0;
}