#include <iostream>
#include <fstream>
#include <iterator>
#include <string.h>
#include <math.h>
#include <vector>
#include <list>
#include <zlib.h>
#include <array>
#include <memory>

#include "../include/lambda/fs.hpp"
#include "../include/lambda/compression.hpp"

int main() {

	lambda::virtualFS vfs;

	std::cout << "Load status: " << vfs.loadSnapshot("fs1.tar") << std::endl;

	vfs.write("/index.html", "hello world!");
	vfs.write("/style.css", "haha this is not a css!");

	auto list = vfs.list();

	for (auto file : list) {
		std::cout << file.name << " : " << file.modified << std::endl;
	}

	std::cout << "Trying to read: " << vfs.read("/testfile.txt") << std::endl;

	std::cout << "saving status: " << vfs.saveSnapshot("snapshot.tar.gz") << std::endl;

	/*std::ifstream textfile("text.txt", std::ifstream::in | std::ifstream::binary);

	std::string content((std::istreambuf_iterator<char>(textfile)), std::istreambuf_iterator<char>());

	auto compressed = lambda::gzCompress(&content, true);

	std::cout << "Compressed size: " << compressed.size() << std::endl;

	auto restored = lambda::gzDecompress(&compressed);

	std::cout << "restored size: " << restored.size() << std::endl;

	std::cout << "Matched? " << (restored == content) << std::endl;*/

	return 0;
}