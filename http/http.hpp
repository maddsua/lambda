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

	class HttpVersion {
		public:
			HttpVersion() {};
			HttpVersion(const std::string& version);

			std::string toString();

			uint16_t major = 1;
			uint16_t minor = 1;
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
			bool has(const std::string& key) const;

			/**
			 * Retrieves a header value, or an empty string if header not present
			*/
			std::string get(const std::string& key) const;

			/**
			 * Retrieves a vector of multivalue header values, or an empty vector if header not present
			*/
			std::vector<std::string> getMultiValue(const std::string& key) const;

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
			std::string stringify() const;

			/**
			 * Returns const reference to internal data store
			*/
			const std::vector<KVtype>& entries() const;
	};

	class URLSearchParams {
		private:
			std::map<std::string, std::string> data;

		public:
			URLSearchParams() {};
			URLSearchParams(const std::string& URLString) { fromHref(URLString); };
			void fromHref(const std::string& URLString);
			bool has(const std::string key) const;
			void set(const std::string key, const std::string& value);
			bool append(const std::string key, const std::string& value);
			std::string get(const std::string key) const;
			void del(const std::string key);
			std::string stringify() const;
			std::vector<KVtype> entries() const;
			size_t length() const;
	};

	class URL {
		private:
			void refresh();
		
		public:
			URL() {};
			URL(const std::string& href);
			void setHref(const std::string& href);
			std::string href() const;
			std::string toHttpPath() const;
			std::string host;
			std::string port = "80";
			std::string protocol;
			std::string pathname = "/";
			URLSearchParams searchParams;
			bool isWWW() const;
	};

	class Request {
		public:

			/**
			 * Request constructor expects to receive only the http header part, i.e. the data before \r\n\r\n sequence. Data chopping is handled by transport/receiveHTTPRequest.
			 * @param httpHeadStream isn't really a thing that ppl commonly refer to as a stream, that's just the way I call vectors containing partial data in this project.
			 * Just throw the http text starting from beginning an up to header end sequence
			*/
			Request();
			Request(const std::vector<uint8_t>& httpHeadStream);
			std::string method;
			HttpVersion httpversion;
			URL url;
			Headers headers;
			std::vector<uint8_t> body;
			std::string text() const;
			void setBodyText(const std::string& text);
			std::vector<uint8_t> dump();
	};

	class Response {
		private:
			uint16_t _statusCode = 200;
			std::string _statusText = "OK";

		public:
			Response() {};
			Response(const uint16_t statusCode);
			Response(const std::vector<KVtype>& headers, const std::string& body);
			Response(const uint16_t statusCode, const std::vector<KVtype>& headers);
			Response(const uint16_t statusCode, const std::vector<KVtype>& headers, const std::string& body);

			Response(const std::vector<uint8_t>& httpHeadStream);

			Response& operator = (const Request& right);

			bool setStatusCode(const uint16_t statusCode);
			void setCustomStatus(const uint16_t statusCode, const std::string& statusText);
			uint16_t statusCode() const;
			Headers headers;
			std::vector<uint8_t> body;
			void setBodyText(const std::string& text);
			std::vector<uint8_t> dump();
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
			bool has(const std::string key) const;

			/**
			 * Sets a cookie
			*/
			void set(const std::string key, const std::string& value);

			/**
			 * Retrieves a cookie
			*/
			std::string get(const std::string key) const;

			/**
			 * Removes a cookie
			*/
			void del(const std::string key);

			/**
			 * Convert to cookie string
			*/
			std::string stringify() const;

			/**
			 * List all key-value pairs
			*/
			std::vector<KVtype> entries() const;

			/**
			 * Convert to Set-Cookie header KV struct. The whole object is considered as a single cookie for this operation
			*/
			KVtype toHeader() const;
	};

	/**
	 * Get status text for provided status code. Returns empty string if not found
	 * 
	 * Will throw an error if status code is not valid or is unknown
	 */	
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
	std::string getExtMimetype(const std::string& mimetype);

	//	Get file extension for mimetype
	std::string getMimetypeExt(const std::string& mimetype);

	//	Get server date and time
	std::string serverDate();
	std::string serverDate(time_t epoch_time);

	//	get server time
	std::string serverTime(time_t timestamp);
	std::string serverTime();
}

#endif
