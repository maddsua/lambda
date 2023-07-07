#include "./vfs.hpp"
#include "../compress/compress.hpp"
#include "../compress/streams.hpp"

#include <fstream>
#include <cstring>
#include <array>

using namespace Lambda;
using namespace Lambda::Compress;
using namespace Lambda::Storage;

struct LVFS2Record {
	size_t nameSize = 0;
	size_t contentSize = 0;
	size_t metdataSize = 0;
	size_t contentSizeCompressed = 0;
	time_t created = 0;
	time_t modified = 0;
};

/*struct LVFS2Footer {
	size_t footerSize;
};*/

void VFS::exportLvfs2(std::ofstream& filestream) {

	std::lock_guard <std::mutex> lock(threadLock);

	for (const auto& vfile : this->vfs) {

		LVFS2Record header;
		header.contentSize = vfile.second.content.size();
		header.created = vfile.second.created;
		header.modified = vfile.second.modified;
		header.metdataSize = 0;
		header.nameSize = vfile.first.size();

		std::vector<uint8_t> recordContent;
		recordContent.insert(recordContent.end(), vfile.first.begin(), vfile.first.end());
		recordContent.insert(recordContent.end(), vfile.second.content.begin(), vfile.second.content.end());

		std::vector<uint8_t> contentCompressed;
		auto opres = brotliCompressBuffer(recordContent, contentCompressed);
		if (opres.isError()) throw Lambda::Error(std::string("brotli compression error: ") + opres.what());

		header.contentSize = recordContent.size();
		header.contentSizeCompressed = contentCompressed.size();

		filestream.write((char*)&header, sizeof(header));
		filestream.write((char*)contentCompressed.data(), contentCompressed.size());
	}
}

void VFS::importLvfs2(std::ifstream& filestream) {

	while (!filestream.eof()) {
		
		LVFS2Record header;

		filestream.read((char*)&header, sizeof(header));
		if (filestream.eof()) break;

		std::vector<uint8_t> contentCompressed;
		contentCompressed.resize(header.contentSizeCompressed);
		filestream.read((char*)contentCompressed.data(), contentCompressed.size());
		if (filestream.gcount() != contentCompressed.size()) throw Lambda::Error("Corrupted vfs2 file: file read size mismatch");

		std::vector<uint8_t> recordContent;
		
		auto opres = brotliDecompressBuffer(contentCompressed, recordContent);
		if (opres.isError()) throw Lambda::Error(std::string("brotli decompression error: ") + opres.what());
		if (recordContent.size() != header.contentSize) throw Lambda::Error("Corrupted vfs2 file: decompressed file size mismatch");

		VirtualFile vfile;
		vfile.created = header.created;
		vfile.modified = header.modified;

		auto vfileName = std::string(recordContent.begin(), recordContent.begin() + header.nameSize);

		if (header.contentSize) {
			vfile.content.insert(vfile.content.end(), recordContent.begin() + header.nameSize, recordContent.begin() + header.contentSize);
		}

		std::lock_guard <std::mutex> lock(threadLock);

		this->vfs[vfileName] = vfile;
	}
}
