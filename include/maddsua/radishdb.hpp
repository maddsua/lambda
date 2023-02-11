#ifndef _maddsua_radish_database
#define _maddsua_radish_database

#include <stdint.h>
#include <string>
#include <vector>
#include <list>
#include <mutex>

namespace maddsua {
	
	class radishDB {
		public:
			struct listing {
				time_t updated = 0;
				time_t accessed = 0;
				size_t size = 0;
				std::string key;
			};

			bool set(std::string key, std::string value, bool replace);
			std::string get(std::string key);
			bool rename(std::string key, std::string newKey);
			bool remove(std::string key);
			bool check(std::string key);

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
			std::mutex threadLock;
			std::list <dbitem> dbdata;
	};

}


#endif