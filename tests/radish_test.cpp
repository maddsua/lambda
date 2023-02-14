#include <iostream>
#include <vector>
#include <thread>

#include "../include/maddsua/radishdb.hpp"


int main() {

	auto db = new maddsua::radishDB;


	/*for (size_t i = 0; i < 150; i++) {
		db->push("item" + std::to_string(i), "value" + std::to_string(i), false);
	}*/
	

	/*db->push("item1", "value0", false);
	db->push("item2", "value1222", false);
	db->push("item3", "value2747474", false);
	db->push("toremove", "00000", false);*/

	//std::cout << db->pull("item3") << std::endl;

	//db->store("db.db");

	db->load("db.db");

	//db->remove("toremove");


	auto list = db->list();

	std::cout << "List size: " << list.size() << std::endl;
	std::cout << "Entry #0: " << list[0].key << ", size: " << list[0].size << std::endl;

	for (auto &&i : list) {
		std::cout << i.key << ", updated: " << i.updated << " data: " << db->pull(i.key) << std::endl;
	}
	

	return 0;
}