/*
	maddsua's lambda
	A C++ HTTP server framework
	2023 https://github.com/maddsua/lambda

	Header: some kind of a key-value database. Definitely not inspired by Redis
*/


#ifndef H_MADDSUA_LAMBDA_RADISHDB
#define H_MADDSUA_LAMBDA_RADISHDB

	#include <stdint.h>
	#include <string>
	#include <vector>
	#include <list>
	#include <mutex>

	namespace lambda {
		
		class localdb {

			public:
				//static const size_t maxFailedReadRecords = 4;

				struct listing {
					time_t updated = 0;
					time_t accessed = 0;
					size_t size = 0;
					std::string key;
				};

				bool set(std::string key, std::string value, bool force);
				bool set(std::string key, std::string value);
				std::string get(std::string key);
				bool rename(std::string key, std::string newKey);
				bool remove(std::string key);
				bool exist(std::string key);

				std::vector <listing> list();
				bool store(std::string path);
				bool load(std::string path);

			private:
				struct dbitem {
					time_t updated = 0;
					time_t accessed = 0;
					std::string key;
					std::string value;
					bool markedForDeletion = false;
				};

				dbitem parseEntry(std::string textBlock);

				std::mutex threadLock;
				std::list <dbitem> dbdata;
		};
	}

#endif