#ifndef __LAMBDA_VFS__
#define __LAMBDA_VFS__

#include "../lambda.hpp"

#include <unordered_map>
#include <string>
#include <vector>
#include <mutex>

namespace Lambda::Storage {

	struct VirtualFileBase {
		time_t modified = 0;
		time_t created = 0;
	};

	struct VirtualFile : public VirtualFileBase {
		std::vector <uint8_t> content;
	};

	struct VirtualFileInfo : VirtualFileBase {
		bool found = true;
		std::string name;
		size_t size;
	};

	enum TarCompressionLayer {
		TARCOMPRESS_NONE = 0,
		TARCOMPRESS_GZIP = 1,
	};

	typedef std::unordered_map <std::string, VirtualFile> VFSFS;

	class VFS {
		private:
			VFSFS vfs;
			std::mutex threadLock;

			void importTar(std::ifstream& filestream);
			void exportTar(std::ofstream& filestream, TarCompressionLayer compression);

			void importLvfs2(std::ifstream& filestream);
			void exportLvfs2(std::ofstream& filestream);

		public:
			Lambda::Error read(const std::string& path, std::vector <uint8_t>& content);
			Lambda::Error write(const std::string& path, const std::vector <uint8_t>& content);
			Lambda::Error remove(const std::string& path);
			Lambda::Error rename(const std::string& path, const std::string& newPath);

			std::vector<VirtualFileInfo> listAll();
			VirtualFileInfo fileInfo(const std::string& path);

			Lambda::Error loadSnapshot(const std::string& path);
			Lambda::Error saveSnapshot(const std::string& path);
	};

};

#endif
