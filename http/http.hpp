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
			Headers(const std::string& text);
			Headers(const std::vector<KVtype>& headers);
			bool has(std::string key);
			void set(std::string key, const std::string& value);
			bool append(std::string key, const std::string& value);
			std::string get(std::string key);
			void del(std::string key);
			std::string stringify();
			std::vector<KVtype> entries();
	};

	class URLSearchParams {
		private:
			HeadersMap data;

		public:
			URLSearchParams();
			URLSearchParams(const std::string& URL);
	};

	//	Set all string characters to lower case
	//	AaA -> aaa
	void stringToLowerCase(std::string& str);
	std::string stringToLowerCase(const std::string& str);

	//	Set all string characters to upper case
	//	aAa -> AAA
	void stringToUpperCase(std::string& str);
	std::string stringToUpperCase(const std::string& str);

	//	Capitalize first character in each word and leave others in lower case
	//	aa-bB -> Aa-Bb
	void stringToTittleCase(std::string& str);
	std::string stringToTittleCase(const std::string& str);

	//	Remove trailing and preceiding whitespace characters (\\r\\n\\t\\s)
	void stringTrim(std::string& str);
	std::string stringTrim(const std::string& str);

	//	Split string into array of substrings separated by token
	std::vector<std::string> stringSplit(const std::string& str, const char* token);
	
}

#endif
