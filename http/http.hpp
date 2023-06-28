#ifndef __LAMBDA_HTTP__
#define __LAMBDA_HTTP__

#include <vector>
#include <string>
#include <unordered_map>

namespace Lambda::HTTP {

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
			Headers() {};
			Headers(const std::string& httpHeaders) { fromHTTP(httpHeaders); };
			void fromHTTP(const std::string& httpHeaders);
			void fromEntries(const std::vector<HTTP::KVtype>& headers);
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
		public:
			Request(std::vector<uint8_t>& httpHead);
			
			std::string method;
			std::string path;
			URLSearchParams searchParams;
			Headers headers;
			std::vector<uint8_t> body;
			std::string text();
	};

	class Response {
		private:
			uint16_t _statusCode = 200;
			std::string _status = "OK";

		public:
			Response();
			Response(const uint16_t statusCode);
			Response(const std::vector<KVtype>& headers, const std::string& body);
			Response(const uint16_t statusCode, const std::vector<KVtype>& headers);
			Response(const uint16_t statusCode, const std::vector<KVtype>& headers, const std::string& body);

			Response& operator = (const Request& right);

			void setStatusCode(const uint16_t statusCode);
			uint16_t statusCode();
			Headers headers;
			std::vector<uint8_t> body;
			void setBodyText(const std::string& text);
			std::vector<uint8_t> dump();
	};

	class URL {
		private:
			void refresh();
		
		public:
			URL();
			URL(const std::string href);
			void setHref(const std::string& href);
			const std::string href();
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

	//	Combine multiple strings
	std::string stringJoin(const std::vector<std::string>& strs, const char* token);

	//	Split string into array of substrings separated by token
	std::vector<std::string> stringSplit(const std::string& str, const char* token);

	//	Get mimetype for file extension
	std::string getExtMimetype(const std::string mimetype);

	//	Get file extension for mimetype
	std::string getMimetypeExt(const std::string mimetype);

	//	Get server date and time
	std::string serverDate();
	std::string serverDate(time_t epoch_time);

	//	get server time
	std::string serverTime(time_t timestamp);
	std::string serverTime();
}

#endif
