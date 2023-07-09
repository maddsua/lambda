#include <iostream>
#include <vector>
#include "../storage/vfs.hpp"


int main(int argc, char const *argv[]) {

	std::cout << "\r\n--- VFS file load test begin --- \r\n";

	auto vfs = Lambda::Storage::VFS();

	auto importResult = vfs.loadSnapshot("tests/data/simpletar.tar.gz");
	if (importResult.isError()) std::cout << importResult.what() << std::endl;

	std::cout << "\r\n--- Listing loaded files --- \r\n";

	auto listing = vfs.listAll();

	for (auto& record : listing) {
		std::cout << "File: " << record.name << " | Size: " << record.size << std::endl;
	}

	auto exportResult = vfs.saveSnapshot("tests/data/simpletar.lvfs2");
	if (exportResult.isError()) std::cout << importResult.what() << std::endl;

	return 0;
}
