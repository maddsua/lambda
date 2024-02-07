#ifndef __LIB_MADDSUA_LAMBDA_EXTRA_VFS__
#define __LIB_MADDSUA_LAMBDA_EXTRA_VFS__

#include <unordered_map>
#include <string>
#include <vector>
#include <mutex>
#include <vector>
#include <optional>

namespace Lambda {

	namespace VFS {

		struct StoredValue {
			std::vector <uint8_t> buffer;
			time_t created = 0;
			time_t modified = 0;
		};

		struct VirtualFile : public StoredValue {
			std::string name;
			std::string text() const noexcept;
			size_t size() const noexcept;
		};

		struct VirtualFileInfo {
			std::string name;
			time_t modified = 0;
			time_t created = 0;
			size_t size = 0;
		};

		struct VFSInfo {
			size_t totalFiles = 0;
			size_t totalSize = 0;
		};

		class Interface {
			private:
				std::unordered_map <std::string, StoredValue> m_data;
				std::mutex m_lock;
				VFSInfo m_info;

			public:

				/**
				 * Get file
				*/
				const std::optional<VirtualFile> read(const std::string& path) noexcept;

				/**
				 * Save, add or owerwrite file in the VFS
				*/
				void write(const std::string& path, const std::vector <uint8_t>& content) noexcept;
				void write(const std::string& path, const std::string& content) noexcept;

				/**
				 * Delete a file
				*/
				void remove(const std::string& path) noexcept;

				/**
				 * Rename a file
				*/
				void move(const std::string& path, const std::string& newPath);

				/**
				 * List all stored files
				*/
				std::vector<VirtualFileInfo> listAll() noexcept;

				/**
				 * Get single file metadata
				*/
				std::optional<VirtualFileInfo> fileInfo(const std::string& path) noexcept;

				/**
				 * Load VFS image.
				 * 
				 * Supported formats are: .tar, .tar.gz, .tgz
				*/
				void loadSnapshot(const std::string& path);

				/**
				 * Save VFS image.
				 * 
				 * Supported formats are: .tar, .tar.gz, .tgz
				*/
				void saveSnapshot(const std::string& path);

				const VFSInfo& stats() const noexcept;
		};
	};

	typedef VFS::Interface VirtualFilesystem;
};

#endif
