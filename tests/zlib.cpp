#include <iostream>
#include <vector>
#include "../compress/compress.hpp"

const std::vector<uint64_t> sizesets = {
	1,
	4,
	17,
	48,
	95,
	775,
	1000,
	25000,
	112534,
	500000,
	1000000,
};

int main(int argc, char const *argv[]) {

	std::cout << "\r\n--- zlib gzip compression/decompression test begin --- \r\n";

	for (auto size : sizesets) {

		std::cout << "\r\n--- " << size << " slots --- \r\n";

		std::string textdata;
		
		for (size_t i = 0; i < size; i++) {
			textdata += "sample" + std::to_string(i) + "\r\n";
		}

		std::vector<uint8_t> binData(textdata.begin(), textdata.end());
		std::vector<uint8_t> compressed;

		auto stream = Lambda::Compress::ZlibStream();
		stream.startCompression();
		stream.compressBuffer(&binData, &compressed);

		/*auto compressResult = Lambda::Compress::brotliCompressBuffer(binData, compressed, 6);
		if (compressResult.isError()) std::cout << compressResult.what() << std::endl;*/

		std::cout << "Original size: " << binData.size() << ", compressed: " << compressed.size() << ", delta: " << ((double)compressed.size() / (double)binData.size()) << std::endl;

		std::vector<uint8_t> restored;

		stream.startDecompression();
		stream.decompressBuffer(&compressed, &restored);

		/*auto decompressResult = Lambda::Compress::brotliDecompressBuffer(compressed, restored);
		if (decompressResult.isError()) std::cout << decompressResult.what() << std::endl;*/

		if (restored != binData) throw std::runtime_error("Compression failed: data does not match. Compressed size: " + std::to_string(compressed.size()));

		std::cout << "Data matched" << std::endl;
	}

	return 0;
}
