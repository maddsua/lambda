#ifndef __LAMBDA_VFS__
#define __LAMBDA_VFS__

#include "../lambda_private.hpp"

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

	/**
	 * Tar compression options
	*/
	enum TarCompressionLayer {
		TARCOMPRESS_NONE = 0,
		TARCOMPRESS_GZIP = 1,
	};

	typedef std::unordered_map <std::string, VirtualFile> VFSFS;

	/**
	 * Virtual filesystem class
	*/
	class VFS {
		private:
			VFSFS vfs;
			std::mutex threadLock;

			void importTar(std::ifstream& filestream);
			void exportTar(std::ofstream& filestream, TarCompressionLayer compression);

			void importLvfs2(std::ifstream& filestream);
			void exportLvfs2(std::ofstream& filestream);

		public:

			/**
			 * Get file contents, errors will be returned as a return value
			*/
			Lambda::Error read(const std::string& path, std::vector <uint8_t>& content);

			/**
			 * Get file contents suppressing error, in case of an error the size of returned vector will be zero
			*/
			std::vector <uint8_t> read(const std::string& path);

			/**
			 * Save, add or owerwrite file in the VFS
			*/
			Lambda::Error write(const std::string& path, const std::vector <uint8_t>& content);

			/**
			 * Delete a file
			*/
			Lambda::Error remove(const std::string& path);

			/**
			 * Rename a file
			*/
			Lambda::Error rename(const std::string& path, const std::string& newPath);

			/**
			 * List all stored files
			*/
			std::vector<VirtualFileInfo> listAll();

			/**
			 * Get single file metadata
			*/
			VirtualFileInfo fileInfo(const std::string& path);

			/**
			 * Load VFS image.
			 * 
			 * Supported formats are: .tar, .tar.gz, .tgz, lvfs2
			*/
			Lambda::Error loadSnapshot(const std::string& path);

			/**
			 * Save VFS image.
			 * 
			 * Supported formats are: .tar, .tar.gz, .tgz, lvfs2
			*/
			Lambda::Error saveSnapshot(const std::string& path);
	};

};

#endif
