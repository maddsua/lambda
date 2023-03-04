#include <iostream>
#include <vector>
#include <thread>

#include "../include/lambda/localdb.hpp"


int main() {

	auto db = new lambda::localdb;

	auto filePresent = db->load("db.db");

	if (!filePresent) {
		db->push("item1", "value0", false);
		db->push("item2", "value1222", false);
		db->push("item3", "value2747474", false);
		db->push("toremove", "00000", false);
	}

	db->remove("toremove");

	if (!filePresent) db->store("db.db");


	auto list = db->list();

	std::cout << "List size: " << list.size() << std::endl;

	for (auto &&i : list) std::cout << i.key << ", updated: " << i.updated << " data: " << db->pull(i.key) << std::endl;
	
	return 0;
}