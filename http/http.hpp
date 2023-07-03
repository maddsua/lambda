#ifndef __LAMBDA_HTTP__
#define __LAMBDA_HTTP__

#include <vector>
#include <string>
#include <map>

namespace Lambda::HTTP {

	struct KVtype {
		std::string key;
		std::string value;
	};

	struct HttpVersion {
		HttpVersion() {};
		HttpVersion(const std::string& version);

		uint16_t major = 0;
		uint16_t minor = 0;
	};

	class Headers {
		private:
			std::vector<KVtype> data;

		public:

			/**
			 * This class provides APIs to perform the necessary operations over http headers
			*/
			Headers() {};
			Headers(const std::string& httpHeaders) { fromHTTP(httpHeaders); };

			/**
			 * Construct Headers object directly from http text
			*/
			void fromHTTP(const std::string& httpHeaders);

			/**
			 * Construct Headers object from key-value vector
			*/
			void fromEntries(const std::vector<HTTP::KVtype>& headers);

			/**
			 * Performs check whether a header is present
			*/
			bool has(const std::string& key);

			/**
			 * Retrieves a header value, or an empty string if header not present
			*/
			std::string get(const std::string& key);

			/**
			 * Retrieves a vector of multivalue header values, or an empty vector if header not present
			*/
			std::vector<std::string> getMultiValue(const std::string& key);

			/**
			 * Sets a header, overwriting if exists
			*/
			void set(const std::string& key, const std::string& value);

			/**
			 * Adds a header if no other with such a key is present. Tries to add without overwriting
			*/
			void append(const std::string& key, const std::string& value);

			/**
			 * Adds a multi-value header. Basically, just append one without any checks
			 * Don't use for regular ones or else you might end up with a ton of duplicating headers. Use put() instead
			*/
			bool appendMultivalue(const std::string& key, const std::string& value);

			/**
			 * Deletes a header. Multi-value ones too
			*/
			void del(const std::string& key);

			/**
			 * Dumps all the headers in http text form (one header per line, separated by newline)
			*/
			std::string stringify();

			/**
			 * Returns const reference to internal data store
			*/
			const std::vector<KVtype>& entries();
	};

	class URLSearchParams {
		private:
			std::map<std::string, std::string> data;

		public:
			URLSearchParams() {};
			URLSearchParams(const std::string& URLString) { fromHref(URLString); };
			void fromHref(const std::string& URLString);
			bool has(const std::string key);
			void set(const std::string key, const std::string& value);
			bool append(const std::string key, const std::string& value);
			std::string get(const std::string key);
			void del(const std::string key);
			std::string stringify();
			std::vector<KVtype> entries();
			size_t length();
	};

	class Request {
		public:

			/**
			 * Request constructor expects to receive only the http header part, i.e. the data before \r\n\r\n sequence. Data chopping is handled by transport/receiveHTTPRequest.
			 * @param httpHeadStream isn't really a thing that ppl commonly refer to as a stream, that's just the way I call vectors containing partial data in this project.
			 * Just throw the http text starting from beginning an up to header end sequence
			*/
			Request(std::vector<uint8_t>& httpHeadStream);
			std::string method;
			std::string path;
			//HttpVersion httpversion;
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
			URL() {};
			URL(const std::string href) { setHref(href) ;};
			void setHref(const std::string& href);
			const std::string href();
			std::string host;
			std::string port;
			std::string protocol;
			std::string pathname;
			URLSearchParams searchParams;
	};

	class Cookies {
		private:
			std::map<std::string, std::string> data;

		public:

			/**
			 * Cookie manipulation class
			 * This object can represent all client's cookies in form of "key-value", or a single one in form of "key-value, directive-value..."
			*/
			Cookies() {};
			Cookies(const std::string& cookies) { fromString(cookies); };
			Cookies(const Request& request) { fromRequest(request); };

			/**
			 * Construct Cookie object from a string like document.cookie
			*/
			void fromString(const std::string& cookies);

			/**
			 * Construct Cookie object from http request directly
			*/
			void fromRequest(const Request& request);

			/**
			 * Checks if cookie is present
			*/
			bool has(const std::string key);

			/**
			 * Sets a cookie
			*/
			void set(const std::string key, const std::string& value);

			/**
			 * Retrieves a cookie
			*/
			std::string get(const std::string key);

			/**
			 * Removes a cookie
			*/
			void del(const std::string key);

			/**
			 * Convert to cookie string
			*/
			std::string stringify();

			/**
			 * List all key-value pairs
			*/
			std::vector<KVtype> entries();

			/**
			 * Convert to Set-Cookie header KV struct. The whole object is considered as a single cookie for this operation
			*/
			KVtype toHeader();
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
