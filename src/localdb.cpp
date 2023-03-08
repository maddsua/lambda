/*
	maddsua's lambda
	A C++ HTTP server framework
	2023 https://github.com/maddsua/lambda
*/


#include <time.h>
#include <fstream>

#include "../include/lambda/util.hpp"
#include "../include/lambda/localdb.hpp"
#include "../include/lambda/fs.hpp"
#include "../include/lambda/compress.hpp"


/*
	About thread-safety. Now I'm just muting everything when writing or reading data.
	There are better ways to do that, but this is what it is for now. Will fix it later

	PS. Actually need to implement some kind of access management, so an item wouldn't be
	deleted at the same time while being read by another thread
*/


bool lambda::localdb::set(std::string key, std::string value, bool force) {

	std::lock_guard <std::mutex> lock (threadLock);

	for (auto& entry : dbdata) {
		if (entry.key == key) {
			if (force) {
				entry.value = value;
				entry.updated = time(nullptr);
			}
			return force;
		} 
	}

	dbitem temp;
		temp.key = key;
		temp.value = value;
		temp.updated = time(nullptr);

	dbdata.push_back(std::move(temp));
	return true;
}

bool lambda::localdb::set(std::string key, std::string value) {
	return set(key, value, false);
}

bool lambda::localdb::exist(std::string key) {
	for (auto entry : dbdata) {
		if (entry.key == key) return true;
	}
	return false;
}

std::string lambda::localdb::get(std::string key) {

	std::lock_guard <std::mutex> lock (threadLock);

	for (auto& entry : dbdata) {
		if (entry.key == key) {
			entry.accessed = time(nullptr);
			return entry.value;
		}
	}
	return {};
}

bool lambda::localdb::rename(std::string key, std::string newKey) {

	std::lock_guard <std::mutex> lock (threadLock);

	//	assign new name
	for (auto& entry : dbdata) {
		if (entry.key == key) {
			entry.key = newKey;
			return true;
		}
	}

	return false;
}

bool lambda::localdb::remove(std::string key) {

	std::lock_guard <std::mutex> lock (threadLock);
	
	for (auto itr = dbdata.begin(); itr != dbdata.end(); itr++) {
		if ((*itr).key == key) {
			dbdata.erase(itr);
			return true;
		}
	}

	return false;
}

std::vector <lambda::localdb::listing> lambda::localdb::list() {

	std::vector <localdb::listing> result;

	for (auto& entry : dbdata) {
		localdb::listing temp;
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
		- updated	(string, ASCI)		[CRLF]
		- accessed	(string, ASCI)		[CRLF]
		- key		(string, base64)	[CRLF]
		- value		(string, base64)	[CRLF]
		- Extra line feed (CRLF)
*/

bool lambda::localdb::store(std::string path) {

	std::ofstream localfile(path, std::ios::binary);

	if (!localfile.is_open()) return false;

	zlibCompressStream stream;
	if (!stream.init(zlibCompressStream::defCompress, zlibCompressStream::header_gz)) return false;

	std::lock_guard <std::mutex> lock (threadLock);

	size_t entryIdx = 0;
	std::string entryText;
	std::vector <uint8_t> entryBin;

	for (auto& entry : dbdata) {

		entryIdx++;
		auto dbStreamEnd = entryIdx >= dbdata.size();

		if (entry.markedForDeletion) continue;

		entryText += std::to_string(entry.updated) + "\r\n";
		entryText += std::to_string(entry.accessed) + "\r\n";
		entryText += b64Encode(&entry.key) + "\r\n";
		entryText += b64Encode(&entry.value) + "\r\n";
		entryText += "\r\n";

		stream.compressChunk((uint8_t*)entryText.data(), entryText.size(), &entryBin, dbStreamEnd);
		if (stream.error()) return false;

		if (entryBin.size()) {
			localfile.write((const char*)entryBin.data(), entryBin.size());
			entryBin.clear();
		}

		entryText.clear();
	}

	localfile.close();
	
	return stream.done();
}

bool lambda::localdb::load(std::string path) {

	std::string rawBinData;

	if (!lambda::fs::readSync(path, &rawBinData)) return false;

	//	decompress
	auto dbstring = lambda::gzDecompress(&rawBinData);

	size_t blit_begin = 0;
	size_t blit_end = 0;
	while ((blit_end = dbstring.find("\r\n\r\n", blit_begin)) != std::string::npos) {

		if (blit_end == blit_begin) continue;

		try {

			auto entryString = dbstring.substr(blit_begin, blit_end - blit_begin + 2);
			dbitem tmpentry;

			size_t row = 0;
			size_t enit_begin = 0;
			size_t enit_end = 0;
			while ((enit_end = entryString.find("\r\n", enit_begin)) != std::string::npos) {

				if (enit_end == enit_begin) continue;

				auto entryRow = entryString.substr(enit_begin, enit_end - enit_begin);

				switch (row) {
					case 0:
						tmpentry.updated = std::stoull(entryRow);
					break;

					case 1:
						tmpentry.accessed = std::stoull(entryRow);
					break;

					case 2:
						tmpentry.key = b64Decode(&entryRow);
					break;

					case 3:
						tmpentry.value = b64Decode(&entryRow);
					break;
					
					default:
						throw "wtf?";
					break;
				}

				row++;
				enit_begin = enit_end;
				enit_begin += 2;
			}

			std::lock_guard <std::mutex> lock (threadLock);

			dbdata.push_back(tmpentry);

		} catch(...) {
			//	db entry failed to load
			//	but we don't really care at the moment
			//	this should not happen in normal operation
			//	the only way is if a file is damaged
			//	need to add a way to notify that something is going on
		}
		
		blit_begin = blit_end;
		blit_begin += 4;
	}

	return true;
}
