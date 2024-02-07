#include <iostream>
#include <unordered_map>

#include "../../lambda.hpp"

using namespace Lambda::Storage;

int main(int argc, char const *argv[]) {

	const std::string tarfileloc = "test.tar";

	Lambda::VirtualFilesystem vfs;

	vfs.write("test.txt", "hello world");
	vfs.write("index.html", "<h1>hello world</h1>");

	vfs.saveSnapshot(tarfileloc);

	return 0;
}
