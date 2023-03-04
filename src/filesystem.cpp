/*
	maddsua's lambda
	A C++ HTTP server framework
	2023 https://github.com/maddsua/lambda
*/

//#include <iostream>
#include <regex>
#include <stdio.h>
#include <dir.h>
#include <dirent.h>

#include <fstream>
#include <array>

#include "../include/lambda/fs.hpp"
#include "../include/lambda/compression.hpp"

/*
	Virtual FS
*/

enum tarfilemode {
	tarfilemode_normal = '0',
	tarfilemode_longLink = 'L',
	tarfilemode_directory = '5'
};

lambda::virtualFS::tarFileEntry lambda::virtualFS::readTarHeader(const uint8_t* block512) {

	tarFileEntry header;
	auto temp = new char[tar_maxFieldSize];

	//	check for ustar format
	memset(temp, 0, tar_maxFieldSize);
	memcpy(temp, block512 + 257, 6);
	header.ustarFormat = !strcmp(temp, "ustar");

	//	get entry name
	memset(temp, 0, tar_maxFieldSize);
	memcpy(temp, block512, 100);
	header.name += temp;

	//	get entry size
	try {
		memset(temp, 0, tar_maxFieldSize);
		memcpy(temp, block512 + 124, 12);
		header.contentSize = std::stoull(temp, nullptr, 8);
	} catch(...) {
		header.contentSize = 0;
	}

	//	get last modification date
	try {
		memset(temp, 0, tar_maxFieldSize);
		memcpy(temp, block512 + 136, 12);
		header.modified = std::stoull(temp, nullptr, 8);
	} catch(...) {
		header.modified = 0;
	}

	//	calculate checksum
	size_t checksumUnsigned = 0;
	int64_t checksumSigned = 0;

	for (size_t i = 0; i < 512; i++) {

		uint8_t value_unsigned = block512[i];
		int8_t value_signed = block512[i];

		if (i >= 148 && i < 156) {
			value_unsigned = 32;
			value_signed = 32;
		}

		checksumUnsigned += value_unsigned;
		checksumSigned + value_signed;
	}
	
	//	validate checksum
	try {
		memset(temp, 0, tar_maxFieldSize);
		memcpy(temp, block512 + 148, 8);
		auto checksum = std::stoll(temp, nullptr, 8);
		if (checksum == checksumSigned || checksum == checksumUnsigned) header.checksumValid = true;
			else throw false;
	} catch(...) {
		header.checksumValid = false;
	}

	//	get record type
	header.type = block512[156];
	
	delete temp;
	return header;
}


int lambda::virtualFS::loadSnapshot(std::string filepath) {

	static const char gzExt[] = ".gz";
	auto isGzipped = (filepath.find(gzExt) == (filepath.size() - strlen(gzExt)));

	std::list <std::string> longLinks;

	std::ifstream archFile(filepath, std::ios::binary);

	if (!archFile.is_open()) return st_not_found;

	std::vector <uint8_t> tarBuffer;
	std::array <uint8_t, zlibDecompressStream::chunkSize> readInBuffer;

	zlibDecompressStream inflateStream;
	
	if (isGzipped) {
		if (!inflateStream.init(zlibDecompressStream::winbit_auto)) return st_zlib_error;
	}

	tarFileEntry tempEntry;

	std::lock_guard <std::mutex> lock (threadLock);

	while (true)  {
		
		while (!archFile.eof() && (tarBuffer.size() < tar_blockSize) && !inflateStream.done()) {

			archFile.read((char*)readInBuffer.data(), readInBuffer.size());

			if (isGzipped) inflateStream.doInflate(readInBuffer.data(), archFile.gcount(), &tarBuffer);
				else tarBuffer.insert(tarBuffer.end(), readInBuffer.begin(), readInBuffer.begin() + archFile.gcount());
		}

		//	check if tar is ended
		auto endOfStream = (tarBuffer.size() < tar_blockSize);
		
		//	get tar header
		if (!tempEntry.fetching && !endOfStream) {

			if (!tarBuffer[0]) break;

			tempEntry = readTarHeader(tarBuffer.data());
			tarBuffer.erase(tarBuffer.begin(), tarBuffer.begin() + tar_blockSize);

			if (tempEntry.contentSize) {
				tempEntry.fetching = true;
				continue;
			}

			tempEntry = {};
			continue;
		}

		//	calculate some offsets so we don't fly into segfault
		auto fetchLeft = (tempEntry.contentSize - tempEntry.content.size());
		auto fetchChunk = fetchLeft > tar_blockSize ? tar_blockSize : fetchLeft;

		//	read actual file contents from tar
		if (tempEntry.fetching && !fetchLeft) {

			//	if it's a LongLink, put it's content into correcponding list
			if (tempEntry.name == "././@LongLink" || tempEntry.type == 'L') {
				//	push long name, reset and go to next entry
				longLinks.push_back(tempEntry.content);
				tempEntry = {};
				continue;
			}

			//	apply LongLink name if possible
			if (longLinks.size()) {
				for (auto itr = longLinks.begin(); itr != longLinks.end(); itr++) {
					auto longname = *itr;
					if (longname.find(tempEntry.name) == 0) {
						tempEntry.name = longname;
						longLinks.erase(itr);
						break;
					}
				}
			}

			//	insert slash at the beginning
			if (tempEntry.name[0] != '/') tempEntry.name.insert(tempEntry.name.begin(), '/');

			//	check that there is no backslashes and reset path to lowercase
			for (auto& character : tempEntry.name) {
				if (character == '\\') character = '/';
				else if (character >= 'A' && character <= 'Z') character += 0x20;
			}


			//	check if file already exists
			bool fileMerged = false;
			for (auto itr = vFiles.begin(); itr != vFiles.end(); itr++) {

				auto& file = (*itr);

				if (file.name == tempEntry.name) {

					fileMerged = true;

					if (tempEntry.modified > file.modified) {
						file.modified = tempEntry.modified;
						file.content = tempEntry.content;
					}
				}
			}

			//	if does not, just push
			if (!fileMerged) {
				vFiles.push_back({
					tempEntry.name,
					tempEntry.content,
					tempEntry.modified
				});
			}
			
			//	reset temp file
			tempEntry = {};
			continue;
		}
		
		//	add block content and move one block forward
		tempEntry.content.insert(tempEntry.content.end(), tarBuffer.begin(), endOfStream ? tarBuffer.end() : (tarBuffer.begin() + fetchChunk));
		if (endOfStream) break;
		tarBuffer.erase(tarBuffer.begin(), tarBuffer.begin() + tar_blockSize);
	}
	
	return 0;
}

std::vector <uint8_t> lambda::virtualFS::writeTarEntry(std::string name, const std::string& content, time_t modified, char type) {

	auto blockIncomplete = tar_blockSize - (content.size() % tar_blockSize);
	auto totalBufferSize = tar_blockSize + (content.size() + blockIncomplete);

	std::vector <uint8_t> record;
		record.resize(totalBufferSize, 0);

	//	start writing header
	if (name[0] == '/') name.erase(0, 1);

	//	entry name
	memcpy(record.data(), name.data(), name.size() < 100 ? name.size() : 99);	

	//	file mode
	memcpy(record.data() + 100, "0100777", 7);

	//	group id
	memcpy(record.data() + 108, "0000000", 7);

	//	owner id
	memcpy(record.data() + 116, "0000000", 7);

	//	file size
	char entrySizeOct[16];
	snprintf(entrySizeOct, sizeof(entrySizeOct), "%011o", content.size());
	memcpy(record.data() + 124, entrySizeOct, 11);

	//	modified
	char entryModifiedOct[16];
	snprintf(entryModifiedOct, sizeof(entryModifiedOct), "%011o", modified);
	memcpy(record.data() + 136, entryModifiedOct, 11);

	//	link indicator / file type
	record.at(157) = type;

	//	UStar indicator
	memcpy(record.data() + 257, "ustar\0", 6);

	// UStar version
	memcpy(record.data() + 263, "00", 2);

	//	cheader checksum
	int64_t headerChecksum = 0;
	for (size_t i = 0; i < tar_blockSize; i++) {

		if (i >= 148 && i < 156) {
			headerChecksum += 32;
			continue;
		}

		headerChecksum += (int8_t)record[i];
	}

	char headerCheckSumOct[16];
	snprintf(headerCheckSumOct, sizeof(headerCheckSumOct), "%07o", headerChecksum);
	memcpy(record.data() + 148, headerCheckSumOct, 7);

	//	copy the file itself
	memcpy(record.data() + tar_blockSize, content.data(), content.size());

	return record;
}

int lambda::virtualFS::saveSnapshot(std::string filepath) {

	static const char gzExt[] = ".gz";
	auto isGzipped = (filepath.find(gzExt) == (filepath.size() - strlen(gzExt)));

	std::vector <uint8_t> tarBuffer;
	std::vector <uint8_t> gzBuffer;

	std::ofstream localfile(filepath, std::ios::binary);
	if (!localfile.is_open()) return st_fs_error;

	zlibCompressStream stream;

	if (isGzipped) {
		stream.init(Z_DEFAULT_COMPRESSION, zlibCompressStream::header_gz);
		if (stream.error()) return st_zlib_error;
	}

	size_t entryIdx = 0;
	for (auto& entry : vFiles) {

		auto fileStreamEnd = (entryIdx + 1) == vFiles.size();

		//	prepare data
		if (entry.name.size() < 100) {

			tarBuffer = writeTarEntry(entry.name, entry.content, entry.modified, tarfilemode_normal);

		} else {

			auto tarLongLink = writeTarEntry("././@LongLink", entry.name, entry.modified, tarfilemode_longLink);
			auto tarActualEntry = writeTarEntry(entry.name, entry.content, entry.modified, tarfilemode_normal);

			tarBuffer.insert(tarBuffer.end(), tarLongLink.begin(), tarLongLink.end());
			tarBuffer.insert(tarBuffer.end(), tarActualEntry.begin(), tarActualEntry.end());
		}

		//	append 2x512 zero bytes at the end
		if (fileStreamEnd) tarBuffer.resize(tarBuffer.size() + (2 * tar_blockSize), 0);

		//	compress data
		if (isGzipped) {

			stream.doDeflate(tarBuffer.data(), tarBuffer.size(), &gzBuffer, fileStreamEnd);
			if (stream.error()) return st_zlib_error;

			localfile.write((const char*)gzBuffer.data(), gzBuffer.size());

		} else {
			localfile.write((const char*)tarBuffer.data(), tarBuffer.size());
		}

		tarBuffer.clear();
		gzBuffer.clear();
		entryIdx++;
	}

	return st_ok;
}

std::string lambda::virtualFS::read(std::string internalFilePath) {

	for (auto entry : vFiles) {
		if (entry.name == internalFilePath)
			return entry.content;
	}

	return {};
}

bool lambda::virtualFS::write(std::string internalFilePath, const std::string& content) {

	if (content.size() > fs_max_fileSize) return false;

	virtualFile temp;
		temp.name = internalFilePath;
		temp.content = content;
		temp.modified= time(nullptr);

	for (auto& entry : vFiles) {
		if (entry.name == internalFilePath) {
			std::lock_guard <std::mutex> lock (threadLock);
			entry = temp;			
			return true;
		}
	}

	vFiles.push_back(temp);

	return true;
}

bool lambda::virtualFS::remove(std::string internalFilePath) {

	std::lock_guard <std::mutex> lock (threadLock);

	for (auto itr = vFiles.begin(); itr != vFiles.end(); itr++) {
		if ((*itr).name == internalFilePath) {
			vFiles.erase(itr);
			return true;
		}
	}

	return false;
}

std::vector <lambda::virtualFS::listEntry> lambda::virtualFS::list() {

	std::vector <lambda::virtualFS::listEntry> result;

	for (auto entry : vFiles) {
		result.push_back({
			entry.name,
			entry.modified
		});
	}

	return result;
}


/*
	Normal filesystem
*/

bool lambda::fs::createTree(std::string& tree) {

	tree = std::regex_replace(tree, std::regex("[\\\\\\/]+"), "\\");

	if (!tree.size()) return false;

	auto createIfDontexist = [](std::string path) {
		auto dir = opendir(path.c_str());
		if (dir) {
			closedir(dir);
			return true;
		}
		if (mkdir(path.c_str())) return false;
		return true;
	};

	auto hierrarchy = tree.find_first_of('\\');
	while (hierrarchy != std::string::npos) {
		if (!createIfDontexist(tree.substr(0, hierrarchy))) return false;
		hierrarchy = tree.find_first_of('\\', hierrarchy + 1);
	}

	return true;
}

bool lambda::fs::writeSync(std::string path, const std::string* data) {

	FILE* binfile = fopen64(path.c_str(), "wb");
	if (!binfile) return false;

	if (fwrite(data->data(), 1, data->size(), binfile) != data->size()) {
		fclose(binfile);
		return false;
	};

	fclose(binfile);

	return true;
}

bool lambda::fs::readSync(std::string path, std::string* dest) {

	FILE* binfile = fopen64(path.c_str(), "rb");
	if (!binfile) return false;

	while (!feof(binfile)) {
		uint8_t fileChunk[LAMBDA_FS_READ_CHUNK];
		size_t bytesRead = fread(fileChunk, 1, LAMBDA_FS_READ_CHUNK, binfile);
		if (ferror(binfile)) return false;
		dest->insert(dest->end(), fileChunk, fileChunk + bytesRead);
	}

	fclose(binfile);

	return true;
}