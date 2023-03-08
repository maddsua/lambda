#include <iostream>

#include "../../include/lambda/fs.hpp"
#include "../../include/lambda/compress.hpp"

int main() {

	lambda::virtualFS origin;

	origin.write("/index.html", "hello world!");
	origin.write("/style.css", "haha this is not a css!");
	origin.write("/hack.js", "let cool = \"python sucks\";\r\nconsole.log(cool.replace('python', 'javascript'));");

	auto saveop = origin.saveSnapshot("origin.tar.gz");
	std::cout << "Origin save: " << ((saveop == lambda::virtualFS::st_ok) ? "OK" : ("failed, code: " + std::to_string(saveop)))  << std::endl;

	lambda::virtualFS mirror;

	auto loadop = mirror.loadSnapshot("origin.tar.gz");
	std::cout << "Mirror load: " << ((loadop == lambda::virtualFS::st_ok) ? "OK" : ("failed, code: " + std::to_string(loadop)))  << std::endl;

	std::cout << "\r\nList of mirror:\r\n";
	auto list = mirror.list();
	for (auto file : list) {
		std::cout << file.name << " : " << file.size << std::endl;
	}

	return 0;
}