#ifndef __LAMBDA_HTTP__
#define __LAMBDA_HTTP__

#include <vector>
#include <string>
#include <unordered_map>

namespace HTTP {

	struct KVtype {
		std::string key;
		std::string value;
	};

	typedef std::unordered_map<std::string, std::string> HeadersMap;
	typedef std::unordered_map<std::string, std::string> SearchQueryMap;

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
			SearchQueryMap data;

		public:
			URLSearchParams();
			URLSearchParams(const std::string& URLString);
			bool has(std::string key);
			void set(std::string key, const std::string& value);
			bool append(std::string key, const std::string& value);
			std::string get(std::string key);
			void del(std::string key);
			std::string stringify();
			std::vector<KVtype> entries();
			size_t length();
	};

	class Request {
		private:
			std::string _method;
			std::string _path;
			URLSearchParams _searchParams;
			Headers _headers;
			std::vector<uint8_t> _body;

		public:
			Request(std::vector<uint8_t>& httpHead);
			void setBody(std::vector<uint8_t>& content);
			void setBody(std::string& content);
			const std::string method();
			const std::string path();
			const URLSearchParams searchParams();
			const Headers headers();
			const std::vector<uint8_t> body();
			const std::string text();
	};

	class Response {
		private:
			uint16_t statusCode = 200;
			std::string status = "OK";
			Headers headers;
			std::vector<uint8_t> body;
			void setStatusCode(const uint16_t code);

		public:
			Response();
			Response(const uint16_t statusCode);
			Response(const std::vector<KVtype>& headers);
			Response(const std::vector<uint8_t>& body);
			Response(const std::string& body);
			Response(const std::vector<KVtype>& headers, const std::string& body);
			Response(const std::vector<KVtype>& headers, const std::vector<uint8_t>& body);
			Response(const uint16_t statusCode, const Headers headers);
			Response(const uint16_t statusCode, const std::string& body);
			Response(const uint16_t statusCode, const std::vector<uint8_t>& body);
			Response(const uint16_t statusCode, const std::vector<KVtype>& headers, const std::string& body);
			Response(const uint16_t statusCode, const std::vector<KVtype>& headers, const std::vector<uint8_t>& body);
			std::vector<uint8_t> dump();
	};

	class URL {
		private:
			void refresh();
		
		public:
			URL();
			URL(const std::string href);
			const std::string href();
			void setHref(const std::string href);
			std::string host;
			std::string port;
			std::string protocol;
			std::string pathname;
			URLSearchParams searchParams;
	};

	//	Get status text for provided status code. Returns empty string if not found
	std::string statusText(const uint16_t statusCode);

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

	//	Checks whether string includes substring
	bool stringIncludes(const std::string& str, const std::string& substr);
	bool stringIncludes(const std::string& str, const std::vector <std::string>& substrs);

	//	Checks whether string ends with substring
	bool stringEndsWith(const std::string& str, const std::string& substr);

	//	Checks whether string starts with substring
	bool stringStartsWith(const std::string& str, const std::string& substr);

	//	Remove trailing and preceiding whitespace characters (\\r\\n\\t\\s)
	void stringTrim(std::string& str);
	std::string stringTrim(const std::string& str);

	//	Split string into array of substrings separated by token
	std::vector<std::string> stringSplit(const std::string& str, const char* token);
	
	//	URL-encode string
	std::string encodeURIComponent(const std::string& str);
	//void encodeURIComponent(std::string& str);
}

#endif
