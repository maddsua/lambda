/*
	maddsua's lambda
	A C++ HTTP server framework
	2023 https://github.com/maddsua/lambda

	Header: Filesystem
*/


/*

	TAR File Format

	Offset	Size	Field

	TAR Header:
	0 		100 	File name
	100 	8 		File mode (octal)
	108 	8 		Owner's numeric user ID (octal)
	116 	8 		Group's numeric user ID (octal)
	124 	12 		File size in bytes (octal)
	136 	12 		Last modification time in numeric Unix time format (octal)
	148 	8 		Checksum for header record
	156 	1 		Link indicator (file type)
	157 	100 	Name of linked file 

	UStar:
	0		156 	(Several fields, same as in the old format)
	156		1 		Type flag
	157 	100 	(Same field as in the old format)
	257 	6 		UStar indicator, "ustar", then NUL
	263 	2 		UStar version, "00"
	265 	32 		Owner user name
	297 	32 		Owner group name
	329 	8 		Device major number
	337 	8 		Device minor number
	345 	155 	Filename prefix


	* Numeric values are encoded in octal numbers using ASCII digits, with leading zeroes.
	For historical reasons, a final NUL or space character should also be used.
	Thus although there are 12 bytes reserved for storing the file size, only 11 octal digits can be stored.
	This gives a maximum file size of 8 gigabytes on archived files.
*/


#ifndef H_MADDSUA_LAMBDA_FILESYSTEM
#define H_MADDSUA_LAMBDA_FILESYSTEM

	#include <string>
	#include <list>
	#include <vector>
	#include <mutex>

	#define LAMBDA_FS_READ_CHUNK	(1048576)

	namespace lambda {

		class virtualFS {
			public:
				static const int st_ok = 0;
				static const int st_unknown_format = 1;
				static const int st_not_found = 2;
				static const int st_fs_error = 3;
				static const int st_zlib_error = -1;
				static const int st_tar_error = -2;

				static const size_t tar_maxFieldSize = 160U;
				static const size_t tar_blockSize = 512U;
				static const size_t fs_max_fileSize = 8589934591ULL;

				struct tarFileEntry {
					std::string name;
					std::string content;
					size_t contentSize = 0;
					time_t modified = 0;
					char type = 0;
					bool checksumValid = false;
					bool ustarFormat = false;
					bool fetching = false;
				};

				struct virtualFile {
					std::string name;
					std::string content;
					time_t modified = 0;
				};

				struct listEntry {
					std::string name;
					time_t modified;
				};

				std::vector <listEntry> list();

				int loadSnapshot(std::string filepath);
				int saveSnapshot(std::string filepath);

				std::string read(std::string internalFilePath);
				bool write(std::string internalFilePath, const std::string& content);
				bool remove(std::string internalFilePath);

			private:
				tarFileEntry readTarHeader(const uint8_t* block512);
				std::vector <uint8_t> writeTarEntry(std::string name, const std::string& content, time_t modified, char type);

				std::list <virtualFile> vFiles;
				
				std::mutex threadLock;
		};
		
		namespace fs {

			bool writeSync(const std::string path, const std::string* data);
			bool readSync(const std::string path, std::string* dest);

			//	Create directories recursively
			bool createTree(std::string& tree);
			
		}
	}

#endif