#include <iostream>
#include <vector>
#include "../storage/vfs.hpp"


int main(int argc, char const *argv[]) {

	puts("VFS file load test begin...");
	auto vfs = Lambda::Storage::VFS();
	auto importResult = vfs.loadSnapshot("tests/data/simpletar.tar.gz");
	if (importResult.isError()) throw std::runtime_error(importResult.what());
	puts("Ok\n");

	puts("Listing loaded files...");
	auto listing = vfs.listAll();
	for (auto& record : listing) {
		std::cout << "File: " << record.name << " | Size: " << record.size << std::endl;
	}
	puts("\n");

	puts("VFS file save test begin...");
	auto exportResult = vfs.saveSnapshot("tests/data/simpletar.lvfs2");
	if (exportResult.isError()) throw std::runtime_error(exportResult.what());
	puts("Ok\n");

	return 0;
}
