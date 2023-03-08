/*
	maddsua's lambda
	A C++ HTTP server framework
	2023 https://github.com/maddsua/lambda

	Header: HTTP core functions
*/


#include <string>
#include <vector>

#ifndef H_MADDSUA_LAMBDA_HTTPCORE
#define H_MADDSUA_LAMBDA_HTTPCORE

	#include <vector>
	#include <string>

	namespace lambda {

		//	Transform string to lower case (all lowercase, x-netlify)
		void toLowerCase(std::string* text);
		std::string toLowerCase(std::string text);

		//	Transform string to upper case (all capitals, X-NETLIFY)
		void toUpperCase(std::string* text);
		std::string toUpperCase(std::string text);

		//	Reset string case like this: x-netlify -> X-Netlify
		void toTitleCase(std::string* text);
		std::string toTitleCase(std::string text);

		//	Removes whitespace and newline characters frome the beginning and the end of a string
		void trimString(std::string* text);
		std::string trimString(std::string text);

		//	Split string into vector of strings by the token (character or substring)
		std::vector <std::string> splitBy(const std::string& source, const std::string token);

		struct stringPair {
			std::string key;
			std::string value;
		};

		class httpHeaders {
			public:
				httpHeaders() {};
				httpHeaders(const std::vector <stringPair>& headers);

				//	Parse http headers from text or separate lines
				size_t parse(const std::string& text);
				size_t parse(const std::vector <std::string>& lines);

				//	Add header, if does not exist
				bool add(std::string header, std::string value);

				//	Set header, overwriting existing
				void set(std::string header, std::string value);

				//	Remove header
				bool remove(std::string header);

				//	Checks if header exist
				bool exists(std::string header);

				//	Get header value
				std::string get(std::string header);

				//	Dump all headers as text, one header per line
				std::string dump();

				//	Dump all headers as a vector of string pairs
				std::vector <stringPair> list();

			private:
				std::vector <stringPair> content;
		};

		class httpSearchQuery {
			public:
				httpSearchQuery() {};
				httpSearchQuery(const std::string& url);

				//	Parse search query from a url
				size_t parse(const std::string& url);

				bool add(stringPair query);

				//	Set query, overwriting existing
				void set(stringPair query);

				//	Remove query
				bool remove(std::string query);

				//	Checks if query exist
				bool exists(std::string query);

				//	Returns query value
				std::string find(std::string query);

				//	Export search query as a string
				std::string dump();

			private:
				std::vector <stringPair> content;
		};

		class jstring {
			public:
				jstring() {};

				//	Creates JS string object from std::string
				jstring(const std::string& ccppstring);

				//	some operators to be happy
				jstring& operator += (const std::string& right) {
					sstring.append(right);
					return *this;
				}
				//char operator [] (size_t i) const { return stlstr[i]; }
				//char& operator [] (size_t i) { return stlstr[i]; }
				
				//	Underlying std::string you have direct access to
				std::string sstring;

				//	Returns true if a substring is found
				bool includes(const std::string& substring);
				bool includes(const std::vector <std::string>& substrings);

				//	True, if ends with a substring
				bool endsWith(const std::string& substring);

				//	True, if starts with a substring
				bool startsWith(const std::string& substring);

				// Transform to UPPER CASE
				void toLowerCase();

				// Transform to Title Case
				void toTitleCase();

				// Transform to lower case
				void toUpperCase();
		};

		//	Get time in the format: Wed, 06 Jan 2023 07:28:00 GMT
		std::string httpTime(time_t epoch_time);
		//	Get the current time in the format: Wed, 06 Jan 2023 07:28:00 GMT
		std::string httpTime();

		/**
		 * Returns mimetype for specified file extension.
		 * Defaults to application/octet-stream
		*/
		std::string mimetype(std::string extension);

		/**
		 * Finds http status text for the provided status code.
		 * (403) => "403 Forbidden"
		 * Defaults to "200 OK"
		*/
		std::string httpStatusString(const unsigned int statusCode);


		struct actionResult {
			bool success;
			std::string cause;
			std::string info;
		};

		struct httpRequest {
			bool success;
			std::vector <std::string> arguments;
			httpHeaders headers;
			std::string body;
		};
	}

#endif