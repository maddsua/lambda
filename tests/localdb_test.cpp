#include <iostream>
#include <vector>
#include <thread>

#include "../include/lambda/localdb.hpp"


int main() {

	auto db = new lambda::localdb;

	auto filePresent = db->load("db.db");

	if (!filePresent) {
		db->set("item1", "value0");
		db->set("item2", "value1222");
		db->set("item3", "value2747474");
		db->set("toremove", "00000");
	}

	db->remove("toremove");

	if (!filePresent) {
		std::cout << db->store("db.db") << std::endl;
	}

	auto list = db->list();

	std::cout << "List size: " << list.size() << std::endl;

	for (auto &&i : list) std::cout << i.key << ", updated: " << i.updated << " data: " << db->get(i.key) << std::endl;
	
	return 0;
}