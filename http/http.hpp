#ifndef __LAMBDA_HTTP__
#define __LAMBDA_HTTP__

#include <vector>
#include <string>
#include <map>
#include <unordered_map>

namespace HTTP {

	struct KVtype {
		std::string key;
		std::string value;
	};

	typedef std::unordered_map<std::string, std::string> HeadersMap;

	class Headers {
		private:
			HeadersMap data;

		public:
			Headers();
			Headers(std::string& text);
			Headers(std::vector<KVtype>& headers);
			bool has(std::string key);
			void set(std::string key, const std::string& value);
			bool append(std::string key, const std::string& value);
			std::string get(std::string key);
			void del(std::string key);
			std::string stringify();
			std::vector<KVtype> entries();
	};

	void stringToLowerCase(std::string& str);
	void stringToUpperCase(std::string& str);
	void stringToTittleCase(std::string& str);
	void stringTrim(std::string& str);
	std::vector<std::string> stringSplit(const std::string& str, const char* token);
	
}

#endif
