#include <time.h>

#include "../include/maddsua/radishdb.hpp"
#include "../include/maddsua/compression.hpp"
#include "../include/maddsua/fs.hpp"
#include "../include/maddsua/base64.hpp"


bool maddsua::radishDB::set(std::string key, std::string value, bool replace) {

	for (auto& entry : dbdata) {
		if (entry.key == key) {
			if (replace) {
				entry.value = value;
				entry.updated = time(nullptr);
			}
			return replace;
		} 
	}

	dbitem temp;
		temp.key = key;
		temp.value = value;
		temp.updated = time(nullptr);

	dbdata.push_back(std::move(temp));
	return true;
}
bool maddsua::radishDB::check(std::string key) {
	for (auto entry : dbdata) {
		if (entry.key == key) return true;
	}
	return false;
}
std::string maddsua::radishDB::get(std::string key) {
	std::lock_guard<std::mutex> lock (threadLock);
	for (auto& entry : dbdata) {
		if (entry.key == key) {
			entry.accessed = time(nullptr);
			return entry.value;
		}
	}
	return {};
}
bool maddsua::radishDB::remove(std::string key) {

	for (auto itr = dbdata.begin(); itr != dbdata.end(); itr++) {
		if ((*itr).key == key) {
			dbdata.erase(itr);
			return true;
		}
	}

	return false;
}
std::vector <maddsua::radishDB::listing> maddsua::radishDB::list() {

	std::vector <radishDB::listing> result;
	std::lock_guard<std::mutex> lock (threadLock);

	for (auto& entry : dbdata) {
		radishDB::listing temp;
			temp.accessed = entry.accessed;
			temp.updated = entry.updated;
			temp.key = entry.key;
			temp.size = entry.value.size();
		result.push_back(std::move(temp));
	}

	return result;
}

/*
	Packet structure:
		- updated (time_t)
		- accessed (time_t)
		- key (string)
		- value (string)
*/

bool maddsua::radishDB::store(std::string path) {

	std::string dbstring;

	{
		std::lock_guard<std::mutex> lock (threadLock);

		for (auto& entry : dbdata) {

			if (entry.markedForDeletion) continue;

			dbstring += std::to_string(entry.updated) + "\r\n";
			dbstring += std::to_string(entry.accessed) + "\r\n";
			dbstring += maddsua::b64Encode(&entry.key) + "\r\n";
			dbstring += maddsua::b64Encode(&entry.value) + "\r\n";
			dbstring += "\r\n";
		}
	}


	auto compressed = lambda::compression::zstdCompress(&dbstring);

	if (!lambda::fs::writeFileSync(path, &compressed)) return false;
	
	return true;
}

bool maddsua::radishDB::load(std::string path) {

	std::string rawBinData;

	if (!lambda::fs::readFileSync(path, &rawBinData)) return false;

	//	decompress
	auto dbstring = lambda::compression::zstdDecompress(&rawBinData);

	size_t blit_begin = 0;
	size_t blit_end = 0;
	while ((blit_end = dbstring.find("\r\n\r\n", blit_begin)) != std::string::npos) {

		if (blit_end == blit_begin) continue;

		try {

			auto entryString = dbstring.substr(blit_begin, blit_end - blit_begin + 2);
			dbitem tempentry;

			size_t row = 0;
			size_t enit_begin = 0;
			size_t enit_end = 0;
			while ((enit_end = entryString.find("\r\n", enit_begin)) != std::string::npos) {

				if (enit_end == enit_begin) continue;

				auto entryRow = entryString.substr(enit_begin, enit_end - enit_begin);

				switch (row) {
					case 0:
						tempentry.updated = std::stoull(entryRow);
					break;

					case 1:
						tempentry.accessed = std::stoull(entryRow);
					break;

					case 2:
						tempentry.key = maddsua::b64Decode(&entryRow);
					break;

					case 3:
						tempentry.value = maddsua::b64Decode(&entryRow);
					break;
					
					default:
						throw "wtf?";
					break;
				}

				row++;
				enit_begin = enit_end;
				enit_begin += 2;
			}

			std::lock_guard<std::mutex> lock (threadLock);
			dbdata.push_back(tempentry);

		} catch(const std::exception& e) {
			//std::cerr << e.what() << '\n';
		}
		
		blit_begin = blit_end;
		blit_begin += 4;
	}

	return true;
}
